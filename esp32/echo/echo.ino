// echo: turns an ESP32-S3 into a vendor-specific USB device, cafe:0001, that
// sends back whatever the host writes to it: Day 17's gadget, in hardware.
// Arduino IDE, Tools menu: Board "ESP32S3 Dev Module",
//   USB Mode "USB-OTG (TinyUSB)", USB CDC On Boot "Disabled".
#include "USB.h"
#include "USBVendor.h"

#if ARDUINO_USB_MODE == 1
#error "Set Tools > USB Mode to USB-OTG (TinyUSB)"
#endif
#if ARDUINO_USB_CDC_ON_BOOT
#error "Set Tools > USB CDC On Boot to Disabled, so the device has one interface"
#endif

USBVendor Vendor;

// Runs after each packet from the host has arrived in Vendor's buffer.
static void onData(void *arg, esp_event_base_t base, int32_t id, void *event_data)
{
  uint8_t buf[64];
  size_t len = Vendor.read(buf, sizeof(buf));
  Vendor.write(buf, len);
  Vendor.flush();                  // send now, not when 64 bytes have piled up
  Serial.printf("echoed %u bytes\n", (unsigned)len);
}

void setup()
{
  Serial.begin(115200);            // log output, on the board's UART port
  USB.VID(0xcafe);
  USB.PID(0x0001);
  USB.manufacturerName("Linux Internals Lab");
  USB.productName("Echo device");
  Vendor.onEvent(ARDUINO_USB_VENDOR_DATA_EVENT, onData);
  Vendor.begin();
  USB.begin();
}

void loop()
{
}
