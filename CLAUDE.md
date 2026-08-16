# music_box_controller_fw

Firmware for a Raspberry Pi Pico (RP2040/RP2350) based music box controller: a 4x4 button matrix plus 3 addressable LED strips.

## Build

CMake + Pico SDK (checked out as a sibling directory at `../pico-sdk`).

```
cmake -S project -B build
cmake --build build
```

## Layout

- `src/main.cpp` — entry point, wires up the button matrix and LED strips.
- `src/button_matrix.h` — `ButtonMatrix<ColumnCount, RowCount>` template, scans the matrix and returns a debounced bitfield.
- `src/ws2812_strip.h` / `src/ws2812.pio` — `Ws2812Strip<PixelCount>` PIO-driven WS2812 strip driver.
- `project/CMakeLists.txt` — build target.
- `diagram.json` — Wokwi simulation wiring. **Not auto-linked to the firmware** — GPIO numbers are duplicated as string literals (`"pico:GP18"`) here and as int literals in `main.cpp`. When pins change in one, update the other by hand.

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

`src/main.cpp` currently instantiates all three as generic 16-pixel `Ws2812Strip<16>` — this needs updating to reflect the real signal names/pixel counts above once finalized.
