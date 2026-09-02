#pragma once

// CFG_TUSB_MCU / CFG_TUSB_OS are injected by the Pico SDK's tinyusb_device
// CMake target (see pico-sdk/src/rp2_common/tinyusb/CMakeLists.txt); TinyUSB
// requires CFG_TUSB_MCU to be defined before this file is processed.
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_PICO
#endif

#define CFG_TUD_ENABLED 1
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE)

#define CFG_TUD_ENDPOINT0_SIZE 64

// Vendor-only device: no CDC/MSC/HID/MIDI classes.
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_HID 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 1

#define CFG_TUD_VENDOR_RX_BUFSIZE 64
#define CFG_TUD_VENDOR_TX_BUFSIZE 64
