#ifndef BUTTON_EVENT_H
#define BUTTON_EVENT_H

#include <array>
#include <cstddef>
#include <cstdint>

enum class ButtonEventType : uint8_t {
	kPress,
	kRelease,
};

struct ButtonEvent {
	uint8_t index; // row * ColumnCount + column, matching ButtonMatrix's bitfield layout
	ButtonEventType type;
};

// Fixed-capacity FIFO ring buffer of button events. Decouples matrix
// scanning (main.cpp) from USB delivery (usb_vendor_device.cpp), which may
// not always be able to send immediately -- e.g. the vendor IN endpoint's
// TX FIFO is still draining a previous write.
template <size_t Capacity>
class ButtonEventQueue {
public:
	// Returns false (dropping the event) if the queue is full.
	bool push(ButtonEvent event) {
		size_t next = (head_ + 1) % Capacity;
		if (next == tail_) return false;
		buffer_[head_] = event;
		head_ = next;
		return true;
	}

	// Returns false (leaving out untouched) if the queue is empty.
	bool pop(ButtonEvent& out) {
		if (head_ == tail_) return false;
		out = buffer_[tail_];
		tail_ = (tail_ + 1) % Capacity;
		return true;
	}

	bool empty() const {
		return head_ == tail_;
	}

private:
	std::array<ButtonEvent, Capacity> buffer_{};
	size_t head_ = 0;
	size_t tail_ = 0;
};

#endif
