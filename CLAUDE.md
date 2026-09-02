# music_box_controller_fw

Firmware for a Raspberry Pi Pico (RP2040/RP2350) based music box controller: a 4x4 button matrix plus 3 addressable LED strips.

## Build

CMake + Pico SDK (checked out as a sibling directory at `../pico-sdk`).

```
cmake -S project -B build
cmake --build build
```

## Layout

- `src/main.cpp` — entry point, wires up the button matrix, LED strips, and the USB vendor device.
- `src/button_matrix.h` — `ButtonMatrix<ColumnCount, RowCount>` template, scans the matrix and returns a debounced bitfield.
- `src/ws2812_strip.h` / `src/ws2812.pio` — `Ws2812Strip<PixelCount>` PIO-driven WS2812 strip driver.
- `src/switch_led_controller.h` — `SwitchLedController<ColumnCount, RowCount>`, drives `SW_LED_CTRL` from the button matrix's debounced bitfield (see behavior below).
- `src/tusb_config.h` — TinyUSB build config: vendor class only (no CDC/MSC/HID/MIDI).
- `src/usb_descriptors.c` / `src/usb_descriptors.h` — device/config/string descriptors, plus the BOS descriptor and MS OS 2.0 descriptor set that make Windows bind `winusb.sys` automatically (no INF needed). See USB vendor device section below.
- `src/usb_vendor_device.h` / `src/usb_vendor_device.cpp` — thin init()/task() wrapper around the TinyUSB device stack, plus the mandatory `tud_*_cb` callbacks (mount/umount/suspend/resume, vendor control transfer, vendor bulk echo).
- `project/CMakeLists.txt` — build target.
- `diagram.json` — Wokwi simulation wiring. **Not auto-linked to the firmware** — GPIO numbers are duplicated as string literals (`"pico:GP18"`) here and as int literals in `main.cpp`. When pins change in one, update the other by hand.

## USB vendor device

The device exposes a single USB vendor-specific (WinUSB) interface — no CDC, so `printf`/stdio no longer goes over USB; it goes out UART instead (`pico_enable_stdio_uart` is on, `pico_enable_stdio_usb` is off in `project/CMakeLists.txt`).

- BOS descriptor advertises one MS OS 2.0 platform capability (`desc_bos` in `src/usb_descriptors.c`), which points Windows at the `desc_ms_os_20` descriptor set for the vendor code request (`kVendorRequestMicrosoft`, handled in `tud_vendor_control_xfer_cb` in `src/usb_vendor_device.cpp`).
- The MS OS 2.0 descriptor set marks the vendor interface WinUSB-compatible and registers a `DeviceInterfaceGUIDs` registry property, so Windows binds `winusb.sys` with no INF file and the device is discoverable from a Win32 app via `SetupDi*`/`CM_*` using that GUID.
- `idVendor`/`idProduct` in `src/usb_descriptors.c` are placeholders (TinyUSB's shared open-source test VID) — replace with values you're licensed to use before shipping. The `DeviceInterfaceGUIDs` GUID in `desc_ms_os_20` is also a placeholder generated for this repo — regenerate it if you fork the firmware.
- The vendor bulk endpoints currently just echo whatever they receive (`tud_vendor_rx_cb`) — proves the data path works; real command/protocol handling is a follow-up.

## Schematic pinout naming

### Button matrix (4x4)

| Signal | GPIO | Direction | Electrical notes |
|---|---|---|---|
| SW_C0 | GP27 | Output | Open-collector — driven low to scan, high-Z otherwise |
| SW_C1 | GP21 | Output | Open-collector — driven low to scan, high-Z otherwise |
| SW_C2 | GP20 | Output | Open-collector — driven low to scan, high-Z otherwise |
| SW_C3 | GP17 | Output | Open-collector — driven low to scan, high-Z otherwise |
| SW_L0 | GP26 | Input | Externally pulled up on the board |
| SW_L1 | GP22 | Input | Externally pulled up on the board |
| SW_L2 | GP19 | Input | Externally pulled up on the board |
| SW_L3 | GP18 | Input | Externally pulled up on the board |

GPIO23-25 are skipped because they're reserved on the Pico W (SPI/onboard LED); GP28 is used by `SW_LED_CTRL` instead of the matrix.

Current `src/button_matrix.h` implementation drives columns push-pull (`GPIO_OUT` + `gpio_put`) and pulls rows down internally (`gpio_pull_down`), which is the inverse polarity of the schematic (open-collector columns + externally pulled-up rows). This is a known divergence to reconcile when the real board is wired up — the matrix scan logic will need columns idle high-Z (or driven high) and rows read active-low.

### LED strips (WS2812)

| Signal | GPIO | Pixel count |
|---|---|---|
| SW_LED_CTRL | GP28 | 64 LEDs (switch matrix backlighting) |
| STRIP_1 | GP15 | TBD |
| STRIP_2 | GP14 | TBD |

`src/main.cpp` instantiates `SW_LED_CTRL` as `Ws2812Strip<64>` (driven through `SwitchLedController`). `STRIP_1`/`STRIP_2` are still generic placeholder `Ws2812Strip<16>` — update once their real pixel counts are finalized.

### SW_LED_CTRL pixel layout (64 pixels around the 4x4 matrix)

Each button has 4 dedicated corner pixels — top-left/bottom-left from the gap to its left, bottom-right/top-right from the gap to its right — never shared with the neighboring button, even though physically adjacent. Each column owns a private, contiguous block of 16 indices (col0 = D0-15, col1 = D16-31, col2 = D32-47, col3 = D48-63). Within a column's block the chain snakes down the left-hand gap (rows 0→3) then back up the right-hand gap (rows 3→0), then continues straight into the next column's block. The physical chain order is simply D0→D63.

For row `r` (0-3) and column `c` (0-3), with block start `B = 16 * c`:

- `TL = B + 2r`
- `BL = B + 2r + 1`
- `BR = B + 14 - 2r`
- `TR = B + 15 - 2r`

| | C0 | C1 | C2 | C3 |
|---|---|---|---|---|
| R0 | 0,1,14,15 | 16,17,30,31 | 32,33,46,47 | 48,49,62,63 |
| R1 | 2,3,12,13 | 18,19,28,29 | 34,35,44,45 | 50,51,60,61 |
| R2 | 4,5,10,11 | 20,21,26,27 | 36,37,42,43 | 52,53,58,59 |
| R3 | 6,7,8,9 | 22,23,24,25 | 38,39,40,41 | 54,55,56,57 |

(cell order is `TL, BL, BR, TR`)

### SW_LED_CTRL behavior

The 4 pixels around a switch always move together as one group. Group color is per-row (defaults match that row's switch color: R0 green, R1 blue, R2 yellow, R3 red) and is editable at runtime via `SwitchLedController::set_row_color(row, RgbColor)`.

Each group has 4 intensity levels (brightness scale of the row color):

| Intensity | Scale |
|---|---|
| Off | 0% |
| Passive | 20% |
| Active | 50% |
| Flash | 100% |

Default/idle state is Passive. On press, a group jumps instantly to Flash (100%), then follows an exponential release curve down to Active (50%) over `release_duration_ms` (default 120ms, `set_release_duration_ms`), settling at Active for as long as the switch stays held. Release — at any point, even mid-curve — returns the group to Passive immediately (no curve). `Off` is not driven automatically; it's available for manual overrides via `set_group_intensity(row, column, LedIntensity::kOff)`.
