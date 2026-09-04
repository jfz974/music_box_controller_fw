#include "usb_vendor_device.h"

#include <cstdio>
#include <cstring>
#include <vector>

#include "button_event.h"
#include "osc_commands.h"
#include "osc_message.h"
#include "tusb.h"
#include "usb_descriptors.h"

namespace usb_vendor_device {

namespace {

constexpr size_t kButtonEventQueueCapacity = 32;
ButtonEventQueue<kButtonEventQueueCapacity> button_event_queue;

// Sends the next queued button event as an OSC message with no arguments
// ("/button/xx/press" or "/button/xx/release", see docs/osc/button-events.md).
// Returns false if there was nothing queued.
bool drain_one_button_event() {
	if (!tud_vendor_mounted()) return false;

	ButtonEvent event;
	if (!button_event_queue.pop(event)) return false;

	char address[24];
	snprintf(address, sizeof(address), "/button/%02u/%s", event.index,
		event.type == ButtonEventType::kPress ? "press" : "release");

	std::vector<uint8_t> bytes = OscMessage(address).serialize();
	tud_vendor_write(bytes.data(), bytes.size());
	tud_vendor_write_flush();
	return true;
}

} // namespace

void init() {
	tusb_rhport_init_t dev_init = {
		.role = TUSB_ROLE_DEVICE,
		.speed = TUSB_SPEED_AUTO,
	};
	tusb_init(0, &dev_init);
}

void task() {
	tud_task();
	drain_one_button_event();
}

void push_button_event(uint8_t index, bool pressed) {
	button_event_queue.push(ButtonEvent{index, pressed ? ButtonEventType::kPress : ButtonEventType::kRelease});
}

} // namespace usb_vendor_device

//--------------------------------------------------------------------+
// TinyUSB device callbacks (mandatory; called from C, so extern "C" linkage)
//--------------------------------------------------------------------+

extern "C" {

void tud_mount_cb(void) {}
void tud_umount_cb(void) {}
void tud_suspend_cb(bool remote_wakeup_en) { (void)remote_wakeup_en; }
void tud_resume_cb(void) {}

// Each vendor bulk OUT transfer is expected to carry exactly one complete
// OSC message (see docs/osc/README.md for the wire format and framing
// assumption). Parses it, dispatches by address (osc_commands.cpp), and
// writes back the reply, if any, on the bulk IN endpoint.
void tud_vendor_rx_cb(uint8_t itf, uint8_t const *buffer, uint16_t bufsize) {
	(void)itf;

	OscMessage request;
	if (OscMessage::parse(buffer, bufsize, request)) {
		OscMessage response;
		// Default empty response: a request with a valid but unhandled
		// address still gets exactly one reply -- its own address, with no
		// arguments -- rather than leaving the host's read hanging.
		if (!osc_commands::dispatch(request, response)) {
			response = OscMessage(request.address());
		}

		std::vector<uint8_t> reply_bytes = response.serialize();
		tud_vendor_write(reply_bytes.data(), reply_bytes.size());
		tud_vendor_write_flush();
	}

#if CFG_TUD_VENDOR_RX_BUFSIZE > 0
	tud_vendor_read_flush();
#endif
}

// Serves the Microsoft OS 2.0 descriptor set requested via the vendor code
// advertised in the BOS descriptor's MS OS 2.0 platform capability.
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
	if (stage != CONTROL_STAGE_SETUP) return true;

	if (request->bmRequestType_bit.type != TUSB_REQ_TYPE_VENDOR) return false;

	if (request->bRequest != kVendorRequestMicrosoft || request->wIndex != 7) return false;

	uint16_t total_len;
	std::memcpy(&total_len, desc_ms_os_20 + 8, 2);
	return tud_control_xfer(rhport, request, (void *)(uintptr_t)desc_ms_os_20, total_len);
}

} // extern "C"
