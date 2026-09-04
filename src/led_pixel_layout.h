#ifndef LED_PIXEL_LAYOUT_H
#define LED_PIXEL_LAYOUT_H

#include <array>

// Pixel indices of the 4 corner LEDs (top-left, bottom-left, bottom-right,
// top-right) around switch (row, column) on SW_LED_CTRL's 64-pixel chain.
// Column owns a private, contiguous 16-pixel block (block start = 16 *
// column); see CLAUDE.md's "SW_LED_CTRL pixel layout" section for the full
// derivation. Shared by SwitchLedController (per-switch flash/passive/
// active) and LedModeController (test/vegas modes).
inline std::array<uint, 4> led_group_pixel_indices(uint row, uint column) {
	uint block_start = 16 * column;
	uint top_left = block_start + 2 * row;
	uint bottom_left = block_start + 2 * row + 1;
	uint bottom_right = block_start + 14 - 2 * row;
	uint top_right = block_start + 15 - 2 * row;
	return {top_left, bottom_left, bottom_right, top_right};
}

#endif
