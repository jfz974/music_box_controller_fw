#ifndef BUTTON_MATRIX_H
#define BUTTON_MATRIX_H

#include <array>
#include <cstdint>

#include "pico/stdlib.h"

class ButtonMatrix {
public:
	static constexpr uint kColumnCount = 4;
	static constexpr uint kRowCount = 4;

	ButtonMatrix(const uint (&output_pins)[kColumnCount], const uint (&input_pins)[kRowCount]);

	uint32_t read_state();

private:
	std::array<uint, kColumnCount> output_pins_;
	std::array<uint, kRowCount> input_pins_;
};

#endif