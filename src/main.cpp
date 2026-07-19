#include <stdio.h>

#include "button_matrix.h"
#include "pico/stdlib.h"

int main() {
	const uint column_pins[4] = {2, 3, 4, 5};
	const uint row_pins[4] = {6, 7, 8, 9};
	uint32_t last_state = 0;

	stdio_init_all();
	ButtonMatrix button_matrix(column_pins, row_pins);

	printf("4x4 matrix ready: cols GP2-5, rows GP6-9\r\n");

	while (true) {
		uint32_t state = button_matrix.read_state();
		uint32_t new_presses = state & ~last_state;

		for (uint row = 0; row < ButtonMatrix::kRowCount; ++row) {
			for (uint column = 0; column < ButtonMatrix::kColumnCount; ++column) {
				uint32_t mask = 1u << (row * ButtonMatrix::kColumnCount + column);
				if ((new_presses & mask) != 0) {
					printf("matrix[%u][%u]\r\n", row, column);
				}
			}
		}

		last_state = state;

		sleep_ms(5);
	}
}