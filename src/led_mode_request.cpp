#include "led_mode_request.h"

namespace led_mode_request {

namespace {
LedMode pending_mode = LedMode::kTest;
bool has_pending = false;
} // namespace

void set(LedMode mode) {
	pending_mode = mode;
	has_pending = true;
}

bool take(LedMode& out) {
	if (!has_pending) return false;
	out = pending_mode;
	has_pending = false;
	return true;
}

} // namespace led_mode_request
