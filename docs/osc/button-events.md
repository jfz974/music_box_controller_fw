# `/button/xx/press`, `/button/xx/release`

Asynchronous notifications sent by the device when a switch matrix button is
pressed or released. Unlike [`/device/name`](device-name.md), these are not
replies to a request — the device sends them on its own, over the vendor
bulk IN endpoint, whenever a press/release edge is detected.

Queued and sent from `usb_vendor_device.cpp` (`push_button_event`,
`ButtonEventQueue` in `src/button_event.h`); pushed from the button matrix
scan loop in `src/main.cpp`.

## Message

| Address | Type tags | Arguments |
|---|---|---|
| `/button/xx/press` | `,` | none |
| `/button/xx/release` | `,` | none |

`xx` is the button's flat index, zero-padded to 2 digits (`00`-`15`):
`index = row * 4 + column`, matching `ButtonMatrix`'s debounced-state
bitfield layout (see `../../CLAUDE.md`'s button matrix pinout table for the
row/column numbering).

Examples: `/button/00/press` (row 0, column 0 pressed), `/button/07/release`
(row 1, column 3 released).
