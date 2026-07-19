#include <stdio.h>

#include "pico/stdlib.h"

int main() {
	const uint button_pin = 15;

	stdio_init_all();
	gpio_init(button_pin);
	gpio_set_dir(button_pin, GPIO_IN);
	gpio_pull_up(button_pin);

	bool last_pressed = gpio_get(button_pin) == 0;
	printf("button ready on GP%u\r\n", button_pin);

	while (true) {
		bool pressed = gpio_get(button_pin) == 0;
		if (pressed != last_pressed) {
			printf("button %s\r\n", pressed ? "pressed" : "released");
			last_pressed = pressed;
		}

		sleep_ms(20);
	}
}