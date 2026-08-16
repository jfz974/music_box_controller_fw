#ifndef SWITCH_LED_CONTROLLER_H
#define SWITCH_LED_CONTROLLER_H

#include <array>
#include <cstdint>

#include "pico/stdlib.h"
#include "ws2812_strip.h"

enum class LedIntensity : uint8_t {
	kOff = 0,
	kPassive,
	kActive,
	kFlash,
};

inline float intensity_scale(LedIntensity intensity) {
	switch (intensity) {
		case LedIntensity::kOff: return 0.00f;
		case LedIntensity::kPassive: return 0.20f;
		case LedIntensity::kActive: return 0.50f;
		case LedIntensity::kFlash: return 1.00f;
	}
	return 0.00f;
}

struct RgbColor {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
};

// Drives SW_LED_CTRL: the 4 pixels around each switch (see CLAUDE.md pixel
// layout) move together as one group, colored per row and dimmed per an
// LedIntensity state driven by that switch's press/release edges.
template <uint ColumnCount, uint RowCount>
class SwitchLedController {
public:
	static constexpr uint kColumnCount = ColumnCount;
	static constexpr uint kRowCount = RowCount;
	static constexpr uint kGroupCount = ColumnCount * RowCount;
	static constexpr uint kPixelCount = kGroupCount * 4;

	explicit SwitchLedController(Ws2812Strip<kPixelCount>& strip, uint32_t flash_duration_ms = 120)
		: strip_(strip), flash_duration_ms_(flash_duration_ms) {
		row_colors_.fill(RgbColor{});
		intensities_.fill(LedIntensity::kPassive);
		flash_start_time_.fill(get_absolute_time());
	}

	// Color of a row's LED group; editable at runtime.
	void set_row_color(uint row, RgbColor color) {
		row_colors_[row] = color;
	}

	RgbColor row_color(uint row) const {
		return row_colors_[row];
	}

	void set_flash_duration_ms(uint32_t duration_ms) {
		flash_duration_ms_ = duration_ms;
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
			} else if (pressed && intensities_[index] == LedIntensity::kFlash &&
				absolute_time_diff_us(flash_start_time_[index], now) >= static_cast<int64_t>(flash_duration_ms_) * 1000) {
				intensities_[index] = LedIntensity::kActive;
			}
		}

		last_button_state_ = button_state;
		render();
	}

private:
	// Pixel indices of the 4 corner LEDs around switch (row, column). Column
	// owns a private 16-pixel block; see CLAUDE.md for the derivation.
	static std::array<uint, 4> pixel_indices(uint row, uint column) {
		uint block_start = 16 * column;
		uint top_left = block_start + 2 * row;
		uint bottom_left = block_start + 2 * row + 1;
		uint bottom_right = block_start + 14 - 2 * row;
		uint top_right = block_start + 15 - 2 * row;
		return {top_left, bottom_left, bottom_right, top_right};
	}

	void render() {
		for (uint row = 0; row < kRowCount; ++row) {
			const RgbColor& color = row_colors_[row];
			for (uint column = 0; column < kColumnCount; ++column) {
				uint index = row * kColumnCount + column;
				float scale = intensity_scale(intensities_[index]);
				uint8_t r = static_cast<uint8_t>(color.r * scale);
				uint8_t g = static_cast<uint8_t>(color.g * scale);
				uint8_t b = static_cast<uint8_t>(color.b * scale);

				for (uint pixel : pixel_indices(row, column)) {
					strip_.set_pixel(pixel, r, g, b);
				}
			}
		}
		strip_.show();
	}

	Ws2812Strip<kPixelCount>& strip_;
	uint32_t flash_duration_ms_;
	uint32_t last_button_state_ = 0;
	std::array<RgbColor, kRowCount> row_colors_;
	std::array<LedIntensity, kGroupCount> intensities_;
	std::array<absolute_time_t, kGroupCount> flash_start_time_;
};

#endif
