// SPDX-License-Identifier: GPL-2.0
/*
 * kbuf: a tiny pipe inside the kernel, at /dev/kbuf.
 * write() appends to a 4 KB buffer; read() takes bytes out and
 * waits while the buffer is empty.
 */
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include "kbuf.h"

#define KBUF_SIZE 4096

static char data[KBUF_SIZE];
static size_t data_len;				/* bytes waiting to be read */
static DEFINE_MUTEX(kbuf_lock);			/* protects data and data_len */
static DECLARE_WAIT_QUEUE_HEAD(kbuf_wait);	/* readers sleep here */
static u32 writes;				/* shown in debugfs */
static struct dentry *debug_dir;

static ssize_t kbuf_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	ssize_t ret;

	mutex_lock(&kbuf_lock);
	while (data_len == 0) {
		mutex_unlock(&kbuf_lock);
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;		/* the caller asked not to wait */
		if (wait_event_interruptible(kbuf_wait, data_len > 0))
			return -ERESTARTSYS;	/* a signal woke us up */
		mutex_lock(&kbuf_lock);		/* another reader may have won */
	}

	ret = min(count, data_len);
	if (copy_to_user(buf, data, ret)) {
		ret = -EFAULT;
	} else {
		memmove(data, data + ret, data_len - ret);	/* drop what was read */
		data_len -= ret;
	}
	mutex_unlock(&kbuf_lock);
	return ret;
}

static ssize_t kbuf_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	ssize_t ret;

	mutex_lock(&kbuf_lock);
	ret = min(count, KBUF_SIZE - data_len);
	if (ret == 0) {
		ret = -ENOSPC;
	} else if (copy_from_user(data + data_len, buf, ret)) {
		ret = -EFAULT;
	} else {
		data_len += ret;
		writes++;
	}
	mutex_unlock(&kbuf_lock);

	wake_up_interruptible(&kbuf_wait);
	return ret;
}

static long kbuf_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret;

	mutex_lock(&kbuf_lock);
	switch (cmd) {
	case KBUF_IOC_CLEAR:
		data_len = 0;
		ret = 0;
		break;
	case KBUF_IOC_USED:
		ret = put_user((int)data_len, (int __user *)arg);
		break;
	default:
		ret = -ENOTTY;		/* "Inappropriate ioctl for device" */
	}
	mutex_unlock(&kbuf_lock);
	return ret;
}

static __poll_t kbuf_poll(struct file *file, poll_table *wait)
{
	poll_wait(file, &kbuf_wait, wait);	/* the queue that signals "readable" */
	return data_len > 0 ? EPOLLIN | EPOLLRDNORM : 0;
}

static const struct file_operations kbuf_fops = {
	.owner		= THIS_MODULE,
	.read		= kbuf_read,
	.write		= kbuf_write,
	.unlocked_ioctl	= kbuf_ioctl,
	.poll		= kbuf_poll,
};

static ssize_t used_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	return sysfs_emit(buf, "%zu\n", data_len);
}
static DEVICE_ATTR_RO(used);

static struct attribute *kbuf_attrs[] = {
	&dev_attr_used.attr,
	NULL,
};
ATTRIBUTE_GROUPS(kbuf);

static struct miscdevice kbuf_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "kbuf",
	.fops	= &kbuf_fops,
	.groups	= kbuf_groups,
};

static int __init kbuf_init(void)
{
	int err = misc_register(&kbuf_dev);

	if (err)
		return err;
	debug_dir = debugfs_create_dir("kbuf", NULL);
	debugfs_create_u32("writes", 0644, debug_dir, &writes);
	return 0;
}

static void __exit kbuf_exit(void)
{
	debugfs_remove_recursive(debug_dir);	/* undo init, in reverse order */
	misc_deregister(&kbuf_dev);
}

module_init(kbuf_init);
module_exit(kbuf_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 11 exercise: kbuf with O_NONBLOCK");
