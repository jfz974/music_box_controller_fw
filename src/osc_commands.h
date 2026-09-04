#pragma once

#include "osc_message.h"

// The set of OSC commands the device understands, dispatched by address.
// Each supported address is documented under docs/osc/ (one file per
// command) alongside docs/osc/README.md for the overall protocol.
namespace osc_commands {

// Looks up request.address() in the command table and, if it matches a
// known command, invokes its handler and fills response. Returns false
// (leaving response untouched) if no command matches the address, in which
// case nothing should be sent back.
bool dispatch(const OscMessage& request, OscMessage& response);

} // namespace osc_commands
