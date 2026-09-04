#pragma once

#include <cstdint>

// Thin wrapper around the TinyUSB device stack for the vendor (WinUSB)
// interface. See usb_descriptors.c for the descriptor set.
namespace usb_vendor_device {

// Starts the TinyUSB device stack. Call once at startup, before the main loop.
void init();

// Pumps the TinyUSB device stack, including draining queued button events
// onto the vendor IN endpoint. Call every iteration of the main loop.
void task();

// Queues a button press/release edge (index = row * ColumnCount + column,
// matching ButtonMatrix's bitfield layout) to be sent as an OSC event
// message ("/button/xx/press" or "/button/xx/release", see
// docs/osc/button-events.md) over the vendor bulk IN endpoint. Safe to call
// before the USB stack is mounted; the event is queued and sent once
// possible, or dropped if the queue is full.
void push_button_event(uint8_t index, bool pressed);

} // namespace usb_vendor_device
