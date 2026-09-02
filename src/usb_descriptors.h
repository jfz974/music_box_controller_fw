#pragma once

#include <stdint.h>

// bRequest code used by tud_vendor_control_xfer_cb to recognize the
// Microsoft OS 2.0 descriptor request (must match VENDOR_REQUEST_MICROSOFT
// referenced from the BOS descriptor's MS OS 2.0 platform capability).
enum {
	kVendorRequestMicrosoft = 1,
};

#ifdef __cplusplus
extern "C" {
#endif

// Microsoft OS 2.0 descriptor set, served in response to a vendor control
// request carrying wIndex == MS_OS_20_DESCRIPTOR_INDEX (7).
extern uint8_t const desc_ms_os_20[];

#ifdef __cplusplus
}
#endif
