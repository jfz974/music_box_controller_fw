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
	}

	uint32_t read_state() {
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

private:
	std::array<uint, kColumnCount> output_pins_;
	std::array<uint, kRowCount> input_pins_;
};

#endif