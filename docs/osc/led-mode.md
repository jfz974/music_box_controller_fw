# `/led/mode`

Selects SW_LED_CTRL's active display mode.

Handled by `handle_led_mode` in `src/osc_commands.cpp`; the mode switch
itself happens in `LedModeController` (`src/led_mode_controller.h`), handed
off from the OSC handler via the `led_mode_request` mailbox
(`src/led_mode_request.h`) since the controller instance lives in
`main.cpp`, not in the USB/OSC code.

## Request

| Address | Type tags | Arguments |
|---|---|---|
| `/led/mode` | `,s` | `mode` (string) — one of `test`, `vegas`, `normal`, `off` |

## Response

| Address | Type tags | Arguments |
|---|---|---|
| `/led/mode` | `,s` | the resolved mode name, echoed back; or `"error"` if the argument is missing or not one of the four modes above |

## Modes

- **`test`** — self-test sweep: walks a single lit (white) pixel across all
  64 pixels of SW_LED_CTRL, looping continuously. This is also the mode the
  device boots into, so every LED gets exercised once at power-up.
  Re-sending `/led/mode test` while already in test mode restarts the sweep
  at pixel 0.
- **`vegas`** — rainbow chase: lights an entire row (all 4 switch groups in
  it) in a shifting rainbow color, sweeps top to bottom, then does the same
  sweeping left to right across columns, then repeats from rows again —
  a circular wave around the matrix.
- **`normal`** — the default per-switch behavior: passive/active/flash
  driven by button press and release (see `src/switch_led_controller.h` and
  its behavior description in `../../CLAUDE.md`).
- **`off`** — all 64 pixels off.
