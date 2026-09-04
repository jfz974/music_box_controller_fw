#include "osc_commands.h"

#include <optional>

#include "led_mode.h"
#include "led_mode_request.h"

namespace osc_commands {

namespace {

// "/device/name" -- see docs/osc/device-name.md
bool handle_device_name(const OscMessage& request, OscMessage& response) {
	(void)request;
	response = OscMessage("/device/name");
	response.add_string("Music Box Controller");
	return true;
}

// "/led/mode" -- see docs/osc/led-mode.md
bool handle_led_mode(const OscMessage& request, OscMessage& response) {
	response = OscMessage("/led/mode");

	std::optional<LedMode> mode;
	if (request.argument_count() >= 1) {
		mode = led_mode_from_string(request.arg_string(0));
	}

	if (!mode) {
		response.add_string("error");
		return true;
	}

	led_mode_request::set(*mode);
	response.add_string(led_mode_to_string(*mode));
	return true;
}

struct Command {
	const char* address;
	bool (*handler)(const OscMessage& request, OscMessage& response);
};

constexpr Command kCommands[] = {
	{"/device/name", handle_device_name},
	{"/led/mode", handle_led_mode},
};

} // namespace

bool dispatch(const OscMessage& request, OscMessage& response) {
	for (const Command& command : kCommands) {
		if (request.address() == command.address) {
			return command.handler(request, response);
		}
	}
	return false;
}

} // namespace osc_commands
