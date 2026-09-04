#include <stdio.h>

#include "button_matrix.h"
#include "color_wheel.h"
#include "led_mode.h"
#include "led_mode_controller.h"
#include "led_mode_request.h"
#include "pico/stdlib.h"
#include "usb_vendor_device.h"
#include "ws2812_strip.h"

namespace {

template <uint StripCount, uint PixelCount>
void run_rainbow_chase(Ws2812Strip<PixelCount> (&strips)[StripCount], uint8_t step) {
	for (uint strip_index = 0; strip_index < StripCount; ++strip_index) {
		uint8_t strip_offset = static_cast<uint8_t>(strip_index * (256 / StripCount));
		for (uint pixel = 0; pixel < PixelCount; ++pixel) {
			uint8_t pos = static_cast<uint8_t>(pixel * (256 / PixelCount) + step + strip_offset);
			uint8_t r, g, b;
			color_wheel(pos, r, g, b);
			strips[strip_index].set_pixel(pixel, r, g, b);
		}
		strips[strip_index].show();
	}
}

} // namespace

int main() {
	const uint column_pins[4] = {27, 21, 20, 17}; // SW_C0, SW_C1, SW_C2, SW_C3
	const uint row_pins[4] = {26, 22, 19, 18}; // SW_L0, SW_L1, SW_L2, SW_L3
	uint32_t last_state = 0;
	using Matrix4x4 = ButtonMatrix<4, 4>;

	stdio_init_all();
	usb_vendor_device::init();
	Matrix4x4 button_matrix(column_pins, row_pins);

	Ws2812Strip<64> sw_led_ctrl(28); // SW_LED_CTRL
	LedModeController<4, 4> led_mode(sw_led_ctrl); // starts in test mode; see docs/osc/led-mode.md
	led_mode.set_row_color(0, RgbColor{0, 255, 0});   // R0 switches are green
	led_mode.set_row_color(1, RgbColor{0, 0, 255});   // R1 switches are blue
	led_mode.set_row_color(2, RgbColor{255, 255, 0}); // R2 switches are yellow
	led_mode.set_row_color(3, RgbColor{255, 0, 0});   // R3 switches are red

	Ws2812Strip<16> led_strips[2] = {
		Ws2812Strip<16>(15), // STRIP_1
		Ws2812Strip<16>(14), // STRIP_2
	};
	for (auto& strip : led_strips) {
		strip.show();
	}

	printf("4x4 matrix ready: SW_C0=GP27 SW_C1=GP21 SW_C2=GP20 SW_C3=GP17, SW_L0=GP26 SW_L1=GP22 SW_L2=GP19 SW_L3=GP18\r\n");
	printf("SW_LED_CTRL ready: GP28 (64 pixels); STRIP_1=GP15, STRIP_2=GP14\r\n");

	uint8_t animation_step = 0;
	absolute_time_t next_animation_time = get_absolute_time();

	while (true) {
		usb_vendor_device::task();

		LedMode requested_mode;
		if (led_mode_request::take(requested_mode)) {
			led_mode.set_mode(requested_mode);
		}

		uint32_t state = button_matrix.read_debounced_state();
		uint32_t new_presses = state & ~last_state;
		uint32_t new_releases = ~state & last_state;

		for (uint row = 0; row < Matrix4x4::kRowCount; ++row) {
			for (uint column = 0; column < Matrix4x4::kColumnCount; ++column) {
				uint8_t index = static_cast<uint8_t>(row * Matrix4x4::kColumnCount + column);
				uint32_t mask = 1u << index;
				if ((new_presses & mask) != 0) {
					printf("matrix[%u][%u]\r\n", row, column);
					usb_vendor_device::push_button_event(index, true);
				} else if ((new_releases & mask) != 0) {
					usb_vendor_device::push_button_event(index, false);
				}
			}
		}

		last_state = state;
		led_mode.update(state);

		if (absolute_time_diff_us(get_absolute_time(), next_animation_time) <= 0) {
			run_rainbow_chase(led_strips, animation_step);
			++animation_step;
			next_animation_time = make_timeout_time_ms(30);
		}

		sleep_ms(5);
	}
}