#!/bin/sh
# Build a USB device out of kernel parts, from the shell (run inside the VM).
# It shows up on the software host controller as cafe:0001, a vendor-specific
# device that loops back whatever the host sends it.
set -e
mount -t configfs none /sys/kernel/config 2>/dev/null || true
cd /sys/kernel/config/usb_gadget
mkdir lab
cd lab
echo 0xcafe > idVendor
echo 0x0001 > idProduct
mkdir strings/0x409
echo "Linux Internals Lab" > strings/0x409/manufacturer
echo "Echo device" > strings/0x409/product
mkdir configs/c.1
mkdir functions/Loopback.0		# one interface, class 0xff, two bulk endpoints
ln -s functions/Loopback.0 configs/c.1/
echo dummy_udc.0 > UDC			# plug it in
echo "gadget plugged in; unplug with: echo > /sys/kernel/config/usb_gadget/lab/UDC"
