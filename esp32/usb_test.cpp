// usb_test: talk to the cafe:0001 echo device from user space with libusb.
// It finds the bulk endpoints the way the kernel driver does, sends a string
// and reads the echo. Works with the ESP32-S3 (esp32/echo) or Day 17's gadget.
// Build: g++ -std=c++14 -Wall -o usb_test usb_test.cpp -lusb-1.0
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

    // Like usb_find_common_endpoints(): the first bulk IN and OUT of interface 0.
    libusb_config_descriptor* config = nullptr;
    if (libusb_get_active_config_descriptor(libusb_get_device(dev), &config) != 0) {
        puts("can't read the configuration descriptor");
        return 1;
    }
    const libusb_interface_descriptor& itf = config->interface[0].altsetting[0];
    unsigned char ep_in = 0, ep_out = 0;
    for (int i = 0; i < itf.bNumEndpoints; ++i) {
        const libusb_endpoint_descriptor& ep = itf.endpoint[i];
        if ((ep.bmAttributes & 3) != LIBUSB_TRANSFER_TYPE_BULK)
            continue;
        if (ep.bEndpointAddress & LIBUSB_ENDPOINT_IN)
            ep_in = ep.bEndpointAddress;
        else
            ep_out = ep.bEndpointAddress;
    }
    printf("interface class 0x%02x, bulk OUT 0x%02x, bulk IN 0x%02x\n",
           itf.bInterfaceClass, ep_out, ep_in);
    libusb_free_config_descriptor(config);      // itf points into it: done with both

    unsigned char out[] = "hello device";
    unsigned char in[64] = {};
    int sent = 0, got = 0;
    int err = libusb_claim_interface(dev, 0);
    if (!err)
        err = libusb_bulk_transfer(dev, ep_out, out, sizeof(out), &sent, 1000);
    if (!err)
        err = libusb_bulk_transfer(dev, ep_in, in, sizeof(in) - 1, &got, 1000);

    if (err)
        printf("failed: %s\n", libusb_error_name(err));
    else
        printf("sent %d bytes, got %d back: %s\n", sent, got, (char*)in);

    libusb_release_interface(dev, 0);
    libusb_close(dev);
    libusb_exit(nullptr);
    return err ? 1 : 0;
}
