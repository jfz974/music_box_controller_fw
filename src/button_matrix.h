#ifndef BUTTON_MATRIX_H
#define BUTTON_MATRIX_H

#include <array>
#include <cstdint>

#include "pico/stdlib.h"

template <uint ColumnCount, uint RowCount>
class ButtonMatrix {
public:
	static constexpr uint kColumnCount = ColumnCount;
	static constexpr uint kRowCount = RowCount;

	ButtonMatrix(const uint (&output_pins)[kColumnCount], const uint (&input_pins)[kRowCount]) {
		static_assert(kColumnCount * kRowCount <= 32, "ButtonMatrix bitfield exceeds uint32_t");

		for (uint column = 0; column < kColumnCount; ++column) {
			output_pins_[column] = output_pins[column];
			gpio_init(output_pins_[column]);
			gpio_set_dir(output_pins_[column], GPIO_OUT);
			gpio_put(output_pins_[column], 0);
		}

		for (uint row = 0; row < kRowCount; ++row) {
			input_pins_[row] = input_pins[row];
			gpio_init(input_pins_[row]);
			gpio_set_dir(input_pins_[row], GPIO_IN);
			gpio_pull_down(input_pins_[row]);
		}

		last_raw_state_ = scan_state();
		debounced_state_ = last_raw_state_;
		last_raw_change_time_ = get_absolute_time();
	}

	uint32_t read_state() {
		return scan_state();
	}

	uint32_t read_debounced_state(uint debounce_ms = 20) {
		uint32_t raw_state = scan_state();
		absolute_time_t now = get_absolute_time();

		if (raw_state != last_raw_state_) {
			last_raw_state_ = raw_state;
			last_raw_change_time_ = now;
		} else if (raw_state != debounced_state_ &&
			absolute_time_diff_us(last_raw_change_time_, now) >= static_cast<int64_t>(debounce_ms) * 1000) {
			debounced_state_ = raw_state;
		}

		return debounced_state_;
	}

private:
	uint32_t scan_state() {
		uint32_t state = 0;

		for (uint column = 0; column < kColumnCount; ++column) {
			gpio_put(output_pins_[column], 1);
			sleep_us(50);

			for (uint row = 0; row < kRowCount; ++row) {
				if (gpio_get(input_pins_[row]) != 0) {
					state |= 1u << (row * kColumnCount + column);
				}
			}

			gpio_put(output_pins_[column], 0);
		}

		return state;
	}

	std::array<uint, kColumnCount> output_pins_;
	std::array<uint, kRowCount> input_pins_;
	uint32_t last_raw_state_ = 0;
	uint32_t debounced_state_ = 0;
	absolute_time_t last_raw_change_time_{};
};

#endif