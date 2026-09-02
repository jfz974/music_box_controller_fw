// USB descriptors for the music box controller's vendor interface.
//
// This device exposes a single vendor-specific (WinUSB) interface and no
// other USB classes. A Microsoft OS 2.0 descriptor set is advertised via the
// BOS descriptor so Windows binds winusb.sys automatically, with no INF file
// needed.

#include "usb_descriptors.h"

#include <string.h>

#include "pico/unique_id.h"
#include "tusb.h"

//--------------------------------------------------------------------+
// Device Descriptor
//--------------------------------------------------------------------+

// TODO: 0xCafe is TinyUSB's shared open-source test VID. Replace idVendor
// (and pick a matching idProduct) with values you are licensed to use
// before shipping this device.
#define kUsbVendorId 0xCafe
#define kUsbProductId 0x4011

tusb_desc_device_t const desc_device = {
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE,
	.bcdUSB = 0x0210, // 2.1+ required for a BOS descriptor

	// Device class/subclass/protocol are left unspecified; the vendor class
	// is declared on the interface descriptor instead (single interface, no
	// IAD needed).
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

	.idVendor = kUsbVendorId,
	.idProduct = kUsbProductId,
	.bcdDevice = 0x0100,

	.iManufacturer = 0x01,
	.iProduct = 0x02,
	.iSerialNumber = 0x03,

	.bNumConfigurations = 0x01,
};

uint8_t const *tud_descriptor_device_cb(void) {
	return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum {
	kItfNumVendor = 0,
	kItfNumTotal,
};

#define kEpNumVendorOut 0x01
#define kEpNumVendorIn 0x81

#define kConfigTotalLen (TUD_CONFIG_DESC_LEN + TUD_VENDOR_DESC_LEN)

uint8_t const desc_configuration[] = {
	// Config number, interface count, string index, total length, attribute, power in mA
	TUD_CONFIG_DESCRIPTOR(1, kItfNumTotal, 0, kConfigTotalLen, 0x00, 100),

	// Interface number, string index, EP Out & IN address, EP size
	TUD_VENDOR_DESCRIPTOR(kItfNumVendor, 4, kEpNumVendorOut, kEpNumVendorIn, 64),
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
	(void)index;
	return desc_configuration;
}

//--------------------------------------------------------------------+
// BOS Descriptor
//--------------------------------------------------------------------+

#define kBosTotalLen (TUD_BOS_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)
#define kMsOs20DescLen 0xB2

uint8_t const desc_bos[] = {
	// total length, number of device capabilities
	TUD_BOS_DESCRIPTOR(kBosTotalLen, 1),

	// Microsoft OS 2.0 platform capability: descriptor set length, vendor code
	TUD_BOS_MS_OS_20_DESCRIPTOR(kMsOs20DescLen, kVendorRequestMicrosoft),
};

uint8_t const *tud_descriptor_bos_cb(void) {
	return desc_bos;
}

// Microsoft OS 2.0 descriptor set: declares the vendor interface WinUSB
// compatible so Windows loads winusb.sys without an INF, then registers a
// device interface GUID an app can use to find/open the device via
// SetupDi*/CM_* APIs. Per MS docs this GUID only needs to be unique to this
// device/driver combination; regenerate it if you fork this firmware.
// https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-os-2-0-descriptors-specification
uint8_t const desc_ms_os_20[] = {
	// Set header: length, type, windows version, total length
	U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000),
	U16_TO_U8S_LE(kMsOs20DescLen),

	// Configuration subset header: length, type, configuration index, reserved, configuration total length
	U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION), 0, 0,
	U16_TO_U8S_LE(kMsOs20DescLen - 0x0A),

	// Function subset header: length, type, first interface, reserved, subset length
	U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), kItfNumVendor, 0,
	U16_TO_U8S_LE(kMsOs20DescLen - 0x0A - 0x08),

	// MS OS 2.0 compatible ID descriptor: length, type, compatible ID, sub-compatible ID
	U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible

	// MS OS 2.0 registry property descriptor: length, type
	U16_TO_U8S_LE(kMsOs20DescLen - 0x0A - 0x08 - 0x08 - 0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
	U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), // wPropertyDataType, wPropertyNameLength
	// PropertyName "DeviceInterfaceGUIDs\0" in UTF-16LE
	'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
	'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00,
	0x00, 0x00,
	U16_TO_U8S_LE(0x0050), // wPropertyDataLength
	// bPropertyData: "{412B3F9E-79D1-4B8C-9C3A-6F0E4B7D2C15}" in UTF-16LE
	'{', 0x00, '4', 0x00, '1', 0x00, '2', 0x00, 'B', 0x00, '3', 0x00, 'F', 0x00, '9', 0x00, 'E', 0x00, '-', 0x00,
	'7', 0x00, '9', 0x00, 'D', 0x00, '1', 0x00, '-', 0x00, '4', 0x00, 'B', 0x00, '8', 0x00, 'C', 0x00, '-', 0x00,
	'9', 0x00, 'C', 0x00, '3', 0x00, 'A', 0x00, '-', 0x00, '6', 0x00, 'F', 0x00, '0', 0x00, 'E', 0x00, '4', 0x00,
	'B', 0x00, '7', 0x00, 'D', 0x00, '2', 0x00, 'C', 0x00, '1', 0x00, '5', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00,
};

TU_VERIFY_STATIC(sizeof(desc_ms_os_20) == kMsOs20DescLen, "Incorrect size");

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

enum {
	kStrIdLangId = 0,
	kStrIdManufacturer,
	kStrIdProduct,
	kStrIdSerial,
	kStrIdVendorInterface,
};

char const *string_desc_arr[] = {
	(const char[]){0x09, 0x04}, // 0: English (0x0409)
	"Music Box",                // 1: Manufacturer
	"Music Box Controller",     // 2: Product
	NULL,                       // 3: filled at runtime from the RP2040 unique board id
	"Music Box Vendor Control", // 4: Vendor interface
};

static uint16_t _desc_str[32 + 1];
static char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
	(void)langid;
	size_t chr_count;

	switch (index) {
		case kStrIdLangId:
			memcpy(&_desc_str[1], string_desc_arr[0], 2);
			chr_count = 1;
			break;

		case kStrIdSerial:
			if (!usbd_serial_str[0]) {
				pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));
			}
			chr_count = strlen(usbd_serial_str);
			for (size_t i = 0; i < chr_count; i++) {
				_desc_str[1 + i] = usbd_serial_str[i];
			}
			break;

		default:
			if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) return NULL;

			const char *str = string_desc_arr[index];
			chr_count = strlen(str);
			size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;
			if (chr_count > max_count) chr_count = max_count;

			for (size_t i = 0; i < chr_count; i++) {
				_desc_str[1 + i] = str[i];
			}
			break;
	}

	_desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

	return _desc_str;
}
