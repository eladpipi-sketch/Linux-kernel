// SPDX-License-Identifier: GPL-2.0
/*
 * Day 17 exercise: usb_echo keeps what it found in probe(), and adds a sysfs
 * file on the interface:   echo hi > /sys/bus/usb/devices/1-1:1.0/send
 */
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#define BUF_SIZE 64

struct echo {
	struct usb_device *udev;
	u8 in_ep, out_ep;		/* endpoint addresses */
	char *buf;			/* USB may DMA from it: never on the stack */
	struct mutex lock;		/* one round trip at a time */
};

static int echo_round_trip(struct usb_interface *intf, const char *msg, size_t len)
{
	struct echo *echo = usb_get_intfdata(intf);
	int sent, got, err;

	guard(mutex)(&echo->lock);
	len = min_t(size_t, len, BUF_SIZE);
	memcpy(echo->buf, msg, len);
	err = usb_bulk_msg(echo->udev, usb_sndbulkpipe(echo->udev, echo->out_ep),
			   echo->buf, len, &sent, 1000);
	if (!err)
		err = usb_bulk_msg(echo->udev, usb_rcvbulkpipe(echo->udev, echo->in_ep),
				   echo->buf, BUF_SIZE, &got, 1000);
	if (!err)
		dev_info(&intf->dev, "sent %d bytes, device echoed %d: %.*s\n",
			 sent, got, got, echo->buf);
	return err;
}

static ssize_t send_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	size_t len = count;
	int err;

	if (len && buf[len - 1] == '\n')	/* echo adds a newline: drop it */
		len--;
	err = echo_round_trip(to_usb_interface(dev), buf, len);
	return err ? err : count;
}
static DEVICE_ATTR_WO(send);

static struct attribute *echo_attrs[] = {
	&dev_attr_send.attr,
	NULL,
};
ATTRIBUTE_GROUPS(echo);

static int echo_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_endpoint_descriptor *in, *out;
	struct echo *echo;
	int err;

	err = usb_find_common_endpoints(intf->cur_altsetting, &in, &out, NULL, NULL);
	if (err)
		return err;

	/* devm_: freed automatically when the driver lets go of the interface */
	echo = devm_kzalloc(&intf->dev, sizeof(*echo), GFP_KERNEL);
	if (!echo)
		return -ENOMEM;
	echo->buf = devm_kmalloc(&intf->dev, BUF_SIZE, GFP_KERNEL);
	if (!echo->buf)
		return -ENOMEM;
	echo->udev = interface_to_usbdev(intf);
	echo->in_ep = in->bEndpointAddress;
	echo->out_ep = out->bEndpointAddress;
	mutex_init(&echo->lock);
	usb_set_intfdata(intf, echo);

	return echo_round_trip(intf, "hello device", sizeof("hello device"));
}

static void echo_disconnect(struct usb_interface *intf)
{
	dev_info(&intf->dev, "unplugged\n");
}

static const struct usb_device_id echo_ids[] = {
	{ USB_DEVICE(0xcafe, 0x0001) },
	{ }
};
MODULE_DEVICE_TABLE(usb, echo_ids);

static struct usb_driver echo_driver = {
	.name		= "usb_echo",
	.id_table	= echo_ids,
	.probe		= echo_probe,
	.disconnect	= echo_disconnect,
	.dev_groups	= echo_groups,		/* created after probe, removed before disconnect */
};
module_usb_driver(echo_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 17 exercise: usb_echo with a send file");
