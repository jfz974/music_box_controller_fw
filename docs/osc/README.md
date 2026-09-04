# OSC over USB vendor

The device accepts [OSC 1.0](https://opensoundcontrol.stanford.edu/spec-1_0.html)
messages over the USB vendor interface's bulk endpoints (see
`../../CLAUDE.md` for the USB vendor device itself). Parsing/serialization
lives in `src/osc_message.h`/`.cpp` (`OscMessage`); command dispatch lives in
`src/osc_commands.h`/`.cpp`.

## Framing

Each vendor bulk OUT transfer is expected to carry exactly one complete OSC
message. OSC bundles (`#bundle`-prefixed packets containing multiple
messages) and messages split across multiple transfers are **not**
supported. A malformed or unrecognized transfer is silently dropped — no
error is sent back.

## Message format

An OSC message is:

1. **Address pattern** — an ASCII string starting with `/` (e.g.
   `/device/name`), null-terminated, then zero-padded so the address plus
   terminator take a multiple of 4 bytes.
2. **Type tag string** — starts with `,`, followed by one character per
   argument, null-terminated and padded the same way as the address. `,`
   alone (no tags after the comma) means no arguments.
3. **Arguments**, encoded in order per their type tag:
   - `i` — int32, big-endian, 4 bytes.
   - `f` — float32, big-endian, 4 bytes.
   - `s` — string, null-terminated, zero-padded to a multiple of 4 bytes
     (same rule as the address/type tags).
   - `b` — blob: a big-endian int32 byte count, then that many raw bytes,
     zero-padded to a multiple of 4 bytes.

Any other type tag is treated as malformed and rejected by `OscMessage::parse`.

## Commands

A command is a request/response pair: the host sends an OSC message on the
vendor bulk OUT endpoint, and the device sends exactly one reply back on the
bulk IN endpoint. If the request's address doesn't match any known command,
the device still replies once, with that same address and no arguments (a
default empty response), rather than leaving the host's read pending.
Malformed input (not parseable as an OSC message at all) gets no reply.

Each supported address is documented in its own file here, and registered
in the command table in `src/osc_commands.cpp`:

- [`/device/name`](device-name.md)
- [`/led/mode`](led-mode.md)

## Events

Unlike commands, an event is sent by the device on its own initiative, not
in reply to a request. They're queued (`ButtonEventQueue` in
`src/button_event.h`) and drained onto the vendor bulk IN endpoint from
`usb_vendor_device::task()`.

- [`/button/xx/press`, `/button/xx/release`](button-events.md)
