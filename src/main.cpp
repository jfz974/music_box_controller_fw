#include <stdio.h>

#include "pico/stdlib.h"

int main() {
	stdio_init_all();

	while (true) {
		printf("music_box_controller_fw alive\r\n");
		sleep_ms(1000);
	}
}