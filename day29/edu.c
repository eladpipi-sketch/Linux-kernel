// SPDX-License-Identifier: GPL-2.0
/* edu: a PCI driver for QEMU's "edu" teaching device (qemu ... -device edu) */
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>

#define EDU_ID			0x00	/* 0x010000ed: version 1.0, then 0xed */
#define EDU_LIVENESS		0x04	/* reads back the inverse of what you write */
#define EDU_FACTORIAL		0x08	/* write n, read back n! */
#define EDU_STATUS		0x20
#define EDU_IRQ_STATUS		0x24	/* why the device interrupted us */
#define EDU_IRQ_ACK		0x64	/* write those bits back to lower the IRQ */
#define EDU_DMA_SRC		0x80
#define EDU_DMA_DST		0x88
#define EDU_DMA_COUNT		0x90
#define EDU_DMA_CMD		0x98

#define STATUS_IRQ_FACTORIAL	0x80	/* interrupt when the factorial is done */
#define DMA_START		0x1
#define DMA_TO_RAM		0x2	/* direction: device -> RAM */
#define DMA_IRQ			0x4	/* interrupt when the transfer is done */
#define EDU_BUF_ADDR		0x40000	/* the device's own 4 KB buffer */
#define EDU_BUF_SIZE		4096

struct edu {
	void __iomem *regs;		/* BAR 0, mapped into kernel space */
	struct mutex lock;		/* one operation at a time */
	struct completion done;		/* completed by the interrupt thread */
	char *buf;			/* DMA buffer, as the CPU sees it */
	dma_addr_t buf_dma;		/* the same buffer, as the device sees it */
	size_t buf_len;
};

/* Top half: hard interrupt context. Acknowledge fast, never sleep. */
static irqreturn_t edu_irq(int irq, void *data)
{
	struct edu *edu = data;
	u32 status = readl(edu->regs + EDU_IRQ_STATUS);

	if (!status)
		return IRQ_NONE;			/* not our device */
	writel(status, edu->regs + EDU_IRQ_ACK);	/* lower the interrupt line */
	return IRQ_WAKE_THREAD;
}

/* Bottom half: runs in a kernel thread, so it may sleep. */
static irqreturn_t edu_irq_thread(int irq, void *data)
{
	struct edu *edu = data;

	complete(&edu->done);
	return IRQ_HANDLED;
}

/* echo 10 > factorial; cat factorial */
static ssize_t factorial_store(struct device *dev, struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct edu *edu = dev_get_drvdata(dev);
	u32 n;
	int err;

	err = kstrtou32(buf, 0, &n);
	if (err)
		return err;
	dev_dbg(dev, "computing %u!\n", n);

	guard(mutex)(&edu->lock);		/* unlocked on every return */
	reinit_completion(&edu->done);
	writel(STATUS_IRQ_FACTORIAL, edu->regs + EDU_STATUS);
	writel(n, edu->regs + EDU_FACTORIAL);	/* the device starts computing */
	if (!wait_for_completion_timeout(&edu->done, HZ))
		return -ETIMEDOUT;
	return count;
}

static ssize_t factorial_show(struct device *dev, struct device_attribute *attr,
			      char *buf)
{
	struct edu *edu = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%u\n", readl(edu->regs + EDU_FACTORIAL));
}
static DEVICE_ATTR_RW(factorial);

/* Run one DMA transfer and sleep until the device interrupts. */
static int edu_dma(struct edu *edu, u64 src, u64 dst, u32 len, u32 direction)
{
	reinit_completion(&edu->done);
	writeq(src, edu->regs + EDU_DMA_SRC);
	writeq(dst, edu->regs + EDU_DMA_DST);
	writeq(len, edu->regs + EDU_DMA_COUNT);
	writeq(DMA_START | DMA_IRQ | direction, edu->regs + EDU_DMA_CMD);
	if (!wait_for_completion_timeout(&edu->done, HZ))
		return -ETIMEDOUT;
	return 0;
}

/* echo text > dma: send it to the device and back; cat dma shows what came back */
static ssize_t dma_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct edu *edu = dev_get_drvdata(dev);
	size_t len = min_t(size_t, count, EDU_BUF_SIZE);
	int err;

	guard(mutex)(&edu->lock);
	memcpy(edu->buf, buf, len);
	err = edu_dma(edu, edu->buf_dma, EDU_BUF_ADDR, len, 0);		/* RAM -> device */
	if (err)
		return err;

	memset(edu->buf, 0, EDU_BUF_SIZE);	/* forget our copy */
	err = edu_dma(edu, EDU_BUF_ADDR, edu->buf_dma, len, DMA_TO_RAM);	/* device -> RAM */
	if (err)
		return err;

	edu->buf_len = len;
	return count;
}

static ssize_t dma_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct edu *edu = dev_get_drvdata(dev);

	guard(mutex)(&edu->lock);
	return sysfs_emit(buf, "%.*s", (int)edu->buf_len, edu->buf);
}
static DEVICE_ATTR_RW(dma);

static struct attribute *edu_attrs[] = {
	&dev_attr_factorial.attr,
	&dev_attr_dma.attr,
	NULL,
};
ATTRIBUTE_GROUPS(edu);

static int edu_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct edu *edu;
	int irq, err;

	edu = devm_kzalloc(&pdev->dev, sizeof(*edu), GFP_KERNEL);
	if (!edu)
		return -ENOMEM;
	mutex_init(&edu->lock);
	init_completion(&edu->done);

	err = pcim_enable_device(pdev);
	if (err)
		return err;

	edu->regs = pcim_iomap_region(pdev, 0, "edu");
	if (IS_ERR(edu->regs))
		return PTR_ERR(edu->regs);

	/* The device can only reach the first 256 MB of RAM: 28 address bits. */
	err = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(28));
	if (err)
		return err;
	edu->buf = dmam_alloc_coherent(&pdev->dev, EDU_BUF_SIZE, &edu->buf_dma,
				       GFP_KERNEL);
	if (!edu->buf)
		return -ENOMEM;

	pci_set_master(pdev);	/* let the device write to RAM: MSIs, later DMA */
	err = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_ALL_TYPES);
	if (err < 0)
		return err;
	irq = pci_irq_vector(pdev, 0);
	err = devm_request_threaded_irq(&pdev->dev, irq, edu_irq, edu_irq_thread,
					IRQF_SHARED, "edu", edu);
	if (err)
		return err;

	pci_set_drvdata(pdev, edu);
	dev_info(&pdev->dev, "id %#010x, irq %d, DMA buffer at bus address %pad\n",
		 readl(edu->regs + EDU_ID), irq, &edu->buf_dma);
	return 0;
}

static void edu_remove(struct pci_dev *pdev)
{
	dev_info(&pdev->dev, "removed\n");
}

static const struct pci_device_id edu_ids[] = {
	{ PCI_DEVICE(0x1234, 0x11e8) },
	{ }
};
MODULE_DEVICE_TABLE(pci, edu_ids);

static struct pci_driver edu_driver = {
	.name		= "edu",
	.id_table	= edu_ids,
	.probe		= edu_probe,
	.remove		= edu_remove,
	.dev_groups	= edu_groups,
};
module_pci_driver(edu_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 29: driver for QEMU's edu PCI device");
