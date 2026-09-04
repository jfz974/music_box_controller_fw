# `/device/name`

Returns the device's product name.

Handled by `handle_device_name` in `src/osc_commands.cpp`.

## Request

| Address | Type tags | Arguments |
|---|---|---|
| `/device/name` | `,` | none |

Any arguments sent with the request are ignored.

## Response

| Address | Type tags | Arguments |
|---|---|---|
| `/device/name` | `,s` | `name` (string) — `"Music Box Controller"` |
