#ifndef LED_MODE_REQUEST_H
#define LED_MODE_REQUEST_H

#include "led_mode.h"

// Single-slot mailbox carrying LED mode change requests from OSC command
// handling (osc_commands.cpp's handle_led_mode, invoked synchronously from
// the vendor RX callback within the main loop's usb_vendor_device::task())
// across to main.cpp, which owns the actual LED hardware and its
// LedModeController. No locking needed: everything in this firmware runs
// cooperatively on one core, so there's no concurrent access to guard against.
namespace led_mode_request {

// Records a mode change request, overwriting any not-yet-applied one.
void set(LedMode mode);

// If a mode change is pending, writes it to out, clears the pending flag,
// and returns true. Otherwise returns false and leaves out untouched.
bool take(LedMode& out);

} // namespace led_mode_request

#endif
