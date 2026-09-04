#ifndef COLOR_WHEEL_H
#define COLOR_WHEEL_H

#include <cstdint>

// Maps 0-255 to a color around the rainbow wheel. Shared by main.cpp's
// STRIP_1/STRIP_2 rainbow chase and LedModeController's vegas mode.
inline void color_wheel(uint8_t pos, uint8_t& r, uint8_t& g, uint8_t& b) {
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

#endif
