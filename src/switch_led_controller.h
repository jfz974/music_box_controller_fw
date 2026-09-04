#ifndef SWITCH_LED_CONTROLLER_H
#define SWITCH_LED_CONTROLLER_H

#include <array>
#include <cmath>
#include <cstdint>

#include "led_pixel_layout.h"
#include "pico/stdlib.h"
#include "ws2812_strip.h"

enum class LedIntensity : uint8_t {
	kOff = 0,
	kPassive,
	kActive,
	kFlash,
};

struct RgbColor {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
};

// Drives SW_LED_CTRL: the 4 pixels around each switch (see
// led_pixel_layout.h / CLAUDE.md pixel layout) move together as one group,
// colored per row and dimmed per an LedIntensity state driven by that
// switch's press/release edges.
//
// On press a group jumps instantly to Flash (100%), then follows an
// exponential release curve down to Active (50%) over release_duration_ms.
// Release — at any point, even mid-curve — drops the group to Passive
// immediately.
template <uint ColumnCount, uint RowCount>
class SwitchLedController {
public:
	static constexpr uint kColumnCount = ColumnCount;
	static constexpr uint kRowCount = RowCount;
	static constexpr uint kGroupCount = ColumnCount * RowCount;
	static constexpr uint kPixelCount = kGroupCount * 4;

	static constexpr float kOffScale = 0.00f;
	static constexpr float kPassiveScale = 0.20f;
	static constexpr float kActiveScale = 0.50f;
	static constexpr float kFlashScale = 1.00f;

	explicit SwitchLedController(Ws2812Strip<kPixelCount>& strip, uint32_t release_duration_ms = 120)
		: strip_(strip), release_duration_ms_(release_duration_ms) {
		row_colors_.fill(RgbColor{});
		intensities_.fill(LedIntensity::kPassive);
		flash_start_time_.fill(get_absolute_time());
		current_scale_.fill(kPassiveScale);
	}

	// Color of a row's LED group; editable at runtime.
	void set_row_color(uint row, RgbColor color) {
		row_colors_[row] = color;
	}

	RgbColor row_color(uint row) const {
		return row_colors_[row];
	}

	// Duration of the release curve from Flash down to Active.
	void set_release_duration_ms(uint32_t duration_ms) {
		release_duration_ms_ = duration_ms;
	}

	void set_group_intensity(uint row, uint column, LedIntensity intensity) {
		intensities_[row * kColumnCount + column] = intensity;
	}

	// Advances the press/release state machine from the button matrix's
	// debounced bitfield (same bit layout as ButtonMatrix::read_debounced_state())
	// and renders the result to the strip.
	void update(uint32_t button_state) {
		absolute_time_t now = get_absolute_time();

		for (uint index = 0; index < kGroupCount; ++index) {
			bool pressed = (button_state & (1u << index)) != 0;
			bool was_pressed = (last_button_state_ & (1u << index)) != 0;

			if (pressed && !was_pressed) {
				intensities_[index] = LedIntensity::kFlash;
				flash_start_time_[index] = now;
			} else if (!pressed && was_pressed) {
				intensities_[index] = LedIntensity::kPassive;
			}

			current_scale_[index] = resolve_scale(index, now);
		}

		last_button_state_ = button_state;
		render();
	}

private:
	// Off/Passive/Active are flat levels. Flash decays exponentially toward
	// Active over release_duration_ms, then settles into the Active state.
	float resolve_scale(uint index, absolute_time_t now) {
		switch (intensities_[index]) {
			case LedIntensity::kOff: return kOffScale;
			case LedIntensity::kPassive: return kPassiveScale;
			case LedIntensity::kActive: return kActiveScale;
			case LedIntensity::kFlash: break;
		}

		int64_t elapsed_ms = absolute_time_diff_us(flash_start_time_[index], now) / 1000;
		if (elapsed_ms >= static_cast<int64_t>(release_duration_ms_)) {
			intensities_[index] = LedIntensity::kActive;
			return kActiveScale;
		}

		float t = static_cast<float>(elapsed_ms) / static_cast<float>(release_duration_ms_);
		float remaining = std::exp(-kReleaseCurveSteepness * t);
		return kActiveScale + (kFlashScale - kActiveScale) * remaining;
	}

	void render() {
		for (uint row = 0; row < kRowCount; ++row) {
			const RgbColor& color = row_colors_[row];
			for (uint column = 0; column < kColumnCount; ++column) {
				uint index = row * kColumnCount + column;
				float scale = current_scale_[index];
				uint8_t r = static_cast<uint8_t>(color.r * scale);
				uint8_t g = static_cast<uint8_t>(color.g * scale);
				uint8_t b = static_cast<uint8_t>(color.b * scale);

				for (uint pixel : led_group_pixel_indices(row, column)) {
					strip_.set_pixel(pixel, r, g, b);
				}
			}
		}
		strip_.show();
	}

	// Higher = drops toward Active faster at the start of the release window.
	static constexpr float kReleaseCurveSteepness = 5.0f;

	Ws2812Strip<kPixelCount>& strip_;
	uint32_t release_duration_ms_;
	uint32_t last_button_state_ = 0;
	std::array<RgbColor, kRowCount> row_colors_;
	std::array<LedIntensity, kGroupCount> intensities_;
	std::array<absolute_time_t, kGroupCount> flash_start_time_;
	std::array<float, kGroupCount> current_scale_;
};

#endif
