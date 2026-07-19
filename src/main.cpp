#include <stdio.h>

#include "pico/stdlib.h"

int main() {
	const uint column_pins[4] = {2, 3, 4, 5};
	const uint row_pins[4] = {6, 7, 8, 9};
	bool last_pressed[4][4] = {};

	stdio_init_all();
	for (uint column = 0; column < 4; ++column) {
		gpio_init(column_pins[column]);
		gpio_set_dir(column_pins[column], GPIO_OUT);
		gpio_put(column_pins[column], 0);
	}

	for (uint row = 0; row < 4; ++row) {
		gpio_init(row_pins[row]);
		gpio_set_dir(row_pins[row], GPIO_IN);
		gpio_pull_down(row_pins[row]);
	}

	printf("4x4 matrix ready: cols GP2-5, rows GP6-9\r\n");

	while (true) {
		for (uint column = 0; column < 4; ++column) {
			gpio_put(column_pins[column], 1);
			sleep_us(50);

			for (uint row = 0; row < 4; ++row) {
				bool pressed = gpio_get(row_pins[row]) != 0;
				if (pressed && !last_pressed[row][column]) {
					printf("matrix[%u][%u]\r\n", row, column);
				}
				last_pressed[row][column] = pressed;
			}

			gpio_put(column_pins[column], 0);
		}

		sleep_ms(5);
	}
}