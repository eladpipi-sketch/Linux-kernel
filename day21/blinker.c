// SPDX-License-Identifier: GPL-2.0
/*
 * blinker: a platform driver for a device that only the device tree knows
 * about. It binds to nodes with compatible = "lab,blinker", takes the GPIO
 * line their led-gpios property names, and toggles it every lab,period-ms.
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/gpio/consumer.h>
#include <linux/property.h>
#include <linux/workqueue.h>

struct blinker {
	struct gpio_desc *led;
	struct delayed_work work;
	u32 period_ms;
	int on;
};

static void blink(struct work_struct *work)
{
	struct blinker *b = container_of(to_delayed_work(work), struct blinker, work);

	b->on = !b->on;
	gpiod_set_value(b->led, b->on);
	schedule_delayed_work(&b->work, msecs_to_jiffies(b->period_ms));
}

static int blinker_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct blinker *b;

	b = devm_kzalloc(dev, sizeof(*b), GFP_KERNEL);		/* freed on unbind */
	if (!b)
		return -ENOMEM;
	b->led = devm_gpiod_get(dev, "led", GPIOD_OUT_LOW);	/* from led-gpios */
	if (IS_ERR(b->led))
		return dev_err_probe(dev, PTR_ERR(b->led), "no led-gpios\n");
	b->period_ms = 500;					/* if the node has no lab,period-ms */
	device_property_read_u32(dev, "lab,period-ms", &b->period_ms);

	INIT_DELAYED_WORK(&b->work, blink);
	platform_set_drvdata(pdev, b);
	schedule_delayed_work(&b->work, 0);
	dev_info(dev, "blinking every %u ms\n", b->period_ms);
	return 0;
}

static void blinker_remove(struct platform_device *pdev)
{
	struct blinker *b = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&b->work);	/* then devm frees b and the GPIO */
}

static const struct of_device_id blinker_ids[] = {
	{ .compatible = "lab,blinker" },
	{ }
};
MODULE_DEVICE_TABLE(of, blinker_ids);

static struct platform_driver blinker_driver = {
	.probe = blinker_probe,
	.remove = blinker_remove,
	.driver = {
		.name = "blinker",
		.of_match_table = blinker_ids,
	},
};
module_platform_driver(blinker_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 21: a platform driver for a device-tree node");
