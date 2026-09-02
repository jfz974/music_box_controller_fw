#pragma once

// Thin wrapper around the TinyUSB device stack for the vendor (WinUSB)
// interface. See usb_descriptors.c for the descriptor set.
namespace usb_vendor_device {

// Starts the TinyUSB device stack. Call once at startup, before the main loop.
void init();

// Pumps the TinyUSB device stack. Call every iteration of the main loop.
void task();

} // namespace usb_vendor_device
