#include <stdio.h>

#include "button_matrix.h"
#include "pico/stdlib.h"
#include "ws2812_strip.h"

namespace {

// Maps 0-255 to a color around the rainbow wheel.
void wheel(uint8_t pos, uint8_t& r, uint8_t& g, uint8_t& b) {
	pos = 255 - pos;
	if (pos < 85) {
		r = 255 - pos * 3;
		g = 0;
		b = pos * 3;
	} else if (pos < 170) {
		pos -= 85;
		r = 0;
		g = pos * 3;
		b = 255 - pos * 3;
	} else {
		pos -= 170;
		r = pos * 3;
		g = 255 - pos * 3;
		b = 0;
	}
}

template <uint StripCount, uint PixelCount>
void run_rainbow_chase(Ws2812Strip<PixelCount> (&strips)[StripCount], uint8_t step) {
	for (uint strip_index = 0; strip_index < StripCount; ++strip_index) {
		uint8_t strip_offset = static_cast<uint8_t>(strip_index * (256 / StripCount));
		for (uint pixel = 0; pixel < PixelCount; ++pixel) {
			uint8_t pos = static_cast<uint8_t>(pixel * (256 / PixelCount) + step + strip_offset);
			uint8_t r, g, b;
			wheel(pos, r, g, b);
			strips[strip_index].set_pixel(pixel, r, g, b);
		}
		strips[strip_index].show();
	}
}

} // namespace

int main() {
	const uint column_pins[4] = {18, 19, 20, 21};
	const uint row_pins[4] = {22, 26, 27, 28};
	uint32_t last_state = 0;
	using Matrix4x4 = ButtonMatrix<4, 4>;

	stdio_init_all();
	Matrix4x4 button_matrix(column_pins, row_pins);

	Ws2812Strip<16> led_strips[3] = {
		Ws2812Strip<16>(10),
		Ws2812Strip<16>(11),
		Ws2812Strip<16>(12),
	};
	for (auto& strip : led_strips) {
		strip.show();
	}

	printf("4x4 matrix ready: cols GP18-21, rows GP22,26-28\r\n");
	printf("3x WS2812 16-pixel strips ready: GP10, GP11, GP12\r\n");

	uint8_t animation_step = 0;
	absolute_time_t next_animation_time = get_absolute_time();

	while (true) {
		uint32_t state = button_matrix.read_debounced_state();
		uint32_t new_presses = state & ~last_state;

		for (uint row = 0; row < Matrix4x4::kRowCount; ++row) {
			for (uint column = 0; column < Matrix4x4::kColumnCount; ++column) {
				uint32_t mask = 1u << (row * Matrix4x4::kColumnCount + column);
				if ((new_presses & mask) != 0) {
					printf("matrix[%u][%u]\r\n", row, column);
				}
			}
		}

		last_state = state;

		if (absolute_time_diff_us(get_absolute_time(), next_animation_time) <= 0) {
			run_rainbow_chase(led_strips, animation_step);
			++animation_step;
			next_animation_time = make_timeout_time_ms(30);
		}

		sleep_ms(5);
	}
}