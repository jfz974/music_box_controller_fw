#ifndef LED_MODE_CONTROLLER_H
#define LED_MODE_CONTROLLER_H

#include "color_wheel.h"
#include "led_mode.h"
#include "led_pixel_layout.h"
#include "pico/stdlib.h"
#include "switch_led_controller.h"
#include "ws2812_strip.h"

// Owns SW_LED_CTRL's active LedMode and renders it every tick. kNormal
// delegates entirely to SwitchLedController (per-switch flash/passive/
// active); the other modes drive the strip directly. Starts in kTest, so
// the switch matrix runs a self-test sweep once at power-up (see
// docs/osc/led-mode.md for the "/led/mode" command that (re)selects a mode
// at runtime).
template <uint ColumnCount, uint RowCount>
class LedModeController {
public:
	static constexpr uint kColumnCount = ColumnCount;
	static constexpr uint kRowCount = RowCount;
	static constexpr uint kPixelCount = ColumnCount * RowCount * 4;

	explicit LedModeController(Ws2812Strip<kPixelCount>& strip) : strip_(strip), normal_controller_(strip) {
		set_mode(LedMode::kTest);
	}

	// Pass-through to the "normal" mode's per-row color, editable regardless
	// of the currently active mode.
	void set_row_color(uint row, RgbColor color) {
		normal_controller_.set_row_color(row, color);
	}

	LedMode mode() const {
		return mode_;
	}

	// Switches mode, resetting that mode's animation state so it always
	// starts from the same point -- e.g. re-selecting kTest restarts the
	// sweep at pixel 0, which is how "/led/mode test" retriggers it.
	void set_mode(LedMode mode) {
		mode_ = mode;

		switch (mode_) {
			case LedMode::kTest:
				test_index_ = 0;
				next_step_time_ = get_absolute_time();
				break;

			case LedMode::kVegas:
				vegas_phase_ = VegasPhase::kRows;
				vegas_index_ = 0;
				vegas_hue_ = 0;
				next_step_time_ = get_absolute_time();
				break;

			case LedMode::kOff:
				strip_.clear();
				strip_.show();
				break;

			case LedMode::kNormal:
				break; // SwitchLedController keeps rendering its own current state
		}
	}

	// Advances the active mode's animation and renders it. button_state is
	// only consulted in kNormal mode (same bitfield layout as
	// ButtonMatrix::read_debounced_state()).
	void update(uint32_t button_state) {
		switch (mode_) {
			case LedMode::kNormal: normal_controller_.update(button_state); break;
			case LedMode::kTest: update_test(); break;
			case LedMode::kVegas: update_vegas(); break;
			case LedMode::kOff: break; // static; already cleared in set_mode()
		}
	}

private:
	enum class VegasPhase { kRows, kColumns };

	static constexpr uint32_t kTestStepMs = 80;
	static constexpr uint32_t kVegasStepMs = 150;
	static constexpr uint8_t kVegasHueStep = 24;

	// Self-test: lights exactly one pixel at a time, walking 0 -> kPixelCount-1
	// on a loop, so every physical LED gets individually verified.
	void update_test() {
		if (absolute_time_diff_us(get_absolute_time(), next_step_time_) > 0) return;

		strip_.clear();
		strip_.set_pixel(test_index_, 255, 255, 255);
		strip_.show();

		test_index_ = (test_index_ + 1) % kPixelCount;
		next_step_time_ = make_timeout_time_ms(kTestStepMs);
	}

	// Vegas: sweeps a rainbow-colored line across all rows, then across all
	// columns, then back to rows -- a circular chase around the matrix.
	void update_vegas() {
		if (absolute_time_diff_us(get_absolute_time(), next_step_time_) > 0) return;

		uint8_t r, g, b;
		color_wheel(vegas_hue_, r, g, b);

		bool sweeping_rows = (vegas_phase_ == VegasPhase::kRows);
		uint line_count = sweeping_rows ? kRowCount : kColumnCount;
		uint cross_count = sweeping_rows ? kColumnCount : kRowCount;

		strip_.clear();
		for (uint cross = 0; cross < cross_count; ++cross) {
			uint row = sweeping_rows ? vegas_index_ : cross;
			uint column = sweeping_rows ? cross : vegas_index_;
			for (uint pixel : led_group_pixel_indices(row, column)) {
				strip_.set_pixel(pixel, r, g, b);
			}
		}
		strip_.show();

		vegas_hue_ = static_cast<uint8_t>(vegas_hue_ + kVegasHueStep);
		if (++vegas_index_ >= line_count) {
			vegas_index_ = 0;
			vegas_phase_ = sweeping_rows ? VegasPhase::kColumns : VegasPhase::kRows;
		}

		next_step_time_ = make_timeout_time_ms(kVegasStepMs);
	}

	Ws2812Strip<kPixelCount>& strip_;
	SwitchLedController<kColumnCount, kRowCount> normal_controller_;

	LedMode mode_ = LedMode::kTest;
	absolute_time_t next_step_time_{};

	uint test_index_ = 0;

	VegasPhase vegas_phase_ = VegasPhase::kRows;
	uint vegas_index_ = 0;
	uint8_t vegas_hue_ = 0;
};

#endif
