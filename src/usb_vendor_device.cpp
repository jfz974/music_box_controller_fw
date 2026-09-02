#include "usb_vendor_device.h"

#include <cstring>

#include "tusb.h"
#include "usb_descriptors.h"

namespace usb_vendor_device {

void init() {
	tusb_rhport_init_t dev_init = {
		.role = TUSB_ROLE_DEVICE,
		.speed = TUSB_SPEED_AUTO,
	};
	tusb_init(0, &dev_init);
}

void task() {
	tud_task();
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

// Echoes any data received on the vendor bulk OUT endpoint back out the
// bulk IN endpoint, just to prove the data path works end to end.
void tud_vendor_rx_cb(uint8_t itf, uint8_t const *buffer, uint16_t bufsize) {
	(void)itf;

	tud_vendor_write(buffer, bufsize);
	tud_vendor_write_flush();

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
