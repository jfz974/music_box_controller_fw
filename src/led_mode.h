#ifndef LED_MODE_H
#define LED_MODE_H

#include <optional>
#include <string>

// SW_LED_CTRL's operating mode, switched at runtime via the "/led/mode" OSC
// command (docs/osc/led-mode.md) and rendered by LedModeController.
enum class LedMode {
	kTest,   // self-test: walks a single lit pixel across the whole strip
	kVegas,  // rainbow row-then-column sweep
	kNormal, // per-switch flash/passive/active driven by button presses
	kOff,    // all pixels off
};

// Parses the string argument of a "/led/mode" OSC request. Returns
// nullopt for anything other than "test"/"vegas"/"normal"/"off".
inline std::optional<LedMode> led_mode_from_string(const std::string& value) {
	if (value == "test") return LedMode::kTest;
	if (value == "vegas") return LedMode::kVegas;
	if (value == "normal") return LedMode::kNormal;
	if (value == "off") return LedMode::kOff;
	return std::nullopt;
}

inline const char* led_mode_to_string(LedMode mode) {
	switch (mode) {
		case LedMode::kTest: return "test";
		case LedMode::kVegas: return "vegas";
		case LedMode::kNormal: return "normal";
		case LedMode::kOff: return "off";
	}
	return "";
}

#endif
