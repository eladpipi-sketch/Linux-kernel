// usb_test: talk to a Raspberry Pi Pico running pico-examples' dev_lowlevel
// firmware (vendor ID changed to 0xcafe) from user space, with libusb.
// It sends a string to bulk endpoint 0x01 (OUT) and reads the echo from 0x82 (IN).
// Build and run in WSL:
//   g++ -std=c++14 -Wall -o usb_test usb_test.cpp -lusb-1.0 && ./usb_test
#include <libusb-1.0/libusb.h>
#include <cstdio>

int main()
{
    libusb_init(nullptr);
    libusb_device_handle* dev = libusb_open_device_with_vid_pid(nullptr, 0xcafe, 0x0001);
    if (!dev) {
        puts("device cafe:0001 not found, or no permission (see the udev rule)");
        return 1;
    }

    unsigned char out[] = "hello device";
    unsigned char in[64] = {};
    int sent = 0, got = 0;
    int err = libusb_claim_interface(dev, 0);
    if (!err)
        err = libusb_bulk_transfer(dev, 0x01, out, sizeof(out), &sent, 1000);
    if (!err)
        err = libusb_bulk_transfer(dev, 0x82, in, sizeof(in) - 1, &got, 1000);

    if (err)
        printf("failed: %s\n", libusb_error_name(err));
    else
        printf("sent %d bytes, got %d back: %s\n", sent, got, (char*)in);

    libusb_release_interface(dev, 0);
    libusb_close(dev);
    libusb_exit(nullptr);
    return err ? 1 : 0;
}
