#ifndef WS2812_STRIP_H
#define WS2812_STRIP_H

#include <array>
#include <cstdint>

#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "ws2812.pio.h"

template <uint PixelCount>
class Ws2812Strip {
public:
	static constexpr uint kPixelCount = PixelCount;

	explicit Ws2812Strip(uint pin, float freq_hz = 800000.f) {
		bool claimed = pio_claim_free_sm_and_add_program_for_gpio_range(
			&ws2812_program, &pio_, &sm_, &offset_, pin, 1, true);
		hard_assert(claimed);

		ws2812_program_init(pio_, sm_, offset_, pin, freq_hz, false);
		pixels_.fill(0);
	}

	void set_pixel(uint index, uint8_t r, uint8_t g, uint8_t b) {
		pixels_[index] = (static_cast<uint32_t>(g) << 16) |
			(static_cast<uint32_t>(r) << 8) |
			static_cast<uint32_t>(b);
	}

	void clear() {
		pixels_.fill(0);
	}

	void show() {
		for (uint32_t pixel_grb : pixels_) {
			pio_sm_put_blocking(pio_, sm_, pixel_grb << 8u);
		}
		sleep_us(60); // WS2812/SK6812 latch (reset) delay
	}

private:
	PIO pio_ = nullptr;
	uint sm_ = 0;
	uint offset_ = 0;
	std::array<uint32_t, PixelCount> pixels_;
};

#endif
