#include <stdio.h>

#include "button_matrix.h"
#include "pico/stdlib.h"
#include "ws2812_strip.h"

int main() {
	const uint column_pins[4] = {2, 3, 4, 5};
	const uint row_pins[4] = {6, 7, 8, 9};
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

	printf("4x4 matrix ready: cols GP2-5, rows GP6-9\r\n");
	printf("3x WS2812 16-pixel strips ready: GP10, GP11, GP12\r\n");

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

		sleep_ms(5);
	}
}