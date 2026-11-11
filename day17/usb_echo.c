// SPDX-License-Identifier: GPL-2.0
/* usb_echo: a driver for the cafe:0001 echo device (gadget.sh, or an ESP32-S3) */
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>

static int echo_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_endpoint_descriptor *in, *out;
	int sent, got, err;
	char *buf;

	err = usb_find_common_endpoints(intf->cur_altsetting, &in, &out, NULL, NULL);
	if (err)
		return err;			/* no bulk in + bulk out pair */

	buf = kmalloc(64, GFP_KERNEL);		/* USB buffers must not live on the stack */
	if (!buf)
		return -ENOMEM;

	strscpy(buf, "hello device", 64);
	err = usb_bulk_msg(udev, usb_sndbulkpipe(udev, out->bEndpointAddress),
			   buf, strlen(buf) + 1, &sent, 1000);
	if (!err)
		err = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, in->bEndpointAddress),
				   buf, 64, &got, 1000);
	if (!err)
		dev_info(&intf->dev, "sent %d bytes, device echoed %d: %.*s\n",
			 sent, got, got, buf);
	kfree(buf);
	return err;
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
};
module_usb_driver(echo_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Day 17: a USB driver for a vendor-specific device");
