#include "osc_message.h"

#include <cstring>

namespace {

// OSC strings (address, type tags, string arguments) are null-terminated
// then zero-padded so the total takes a multiple of 4 bytes.
size_t padded_size(size_t content_len) {
	return ((content_len + 1 + 3) / 4) * 4;
}

void append_padded(std::vector<uint8_t>& out, const char* content, size_t content_len) {
	out.insert(out.end(), content, content + content_len);
	out.resize(out.size() + (padded_size(content_len) - content_len), 0);
}

// OSC numeric wire values are big-endian; RP2040/RP2350 are little-endian.
uint32_t swap_bytes(uint32_t value) {
	return ((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8) | ((value & 0x00FF0000u) >> 8) |
		((value & 0xFF000000u) >> 24);
}

} // namespace

bool OscMessage::parse(const uint8_t* data, size_t size, OscMessage& out) {
	if (size == 0 || data[0] != '/') return false;

	size_t address_len = 0;
	while (address_len < size && data[address_len] != '\0') ++address_len;
	if (address_len == size) return false; // no null terminator found

	size_t pos = padded_size(address_len);
	if (pos > size) return false;

	out = OscMessage(std::string(reinterpret_cast<const char*>(data), address_len));

	if (pos >= size || data[pos] != ',') return false;

	size_t tags_len = 0;
	while (pos + tags_len < size && data[pos + tags_len] != '\0') ++tags_len;
	if (pos + tags_len == size) return false;

	out.type_tags_ = std::string(reinterpret_cast<const char*>(data + pos), tags_len);
	pos += padded_size(tags_len);
	if (pos > size) return false;

	for (size_t i = 1; i < out.type_tags_.size(); ++i) {
		switch (out.type_tags_[i]) {
			case 'i':
			case 'f': {
				if (pos + 4 > size) return false;
				Argument arg{out.arg_data_.size(), 4};
				out.arg_data_.insert(out.arg_data_.end(), data + pos, data + pos + 4);
				out.arguments_.push_back(arg);
				pos += 4;
				break;
			}

			case 's': {
				size_t str_len = 0;
				while (pos + str_len < size && data[pos + str_len] != '\0') ++str_len;
				if (pos + str_len == size) return false;

				Argument arg{out.arg_data_.size(), str_len};
				out.arg_data_.insert(out.arg_data_.end(), data + pos, data + pos + str_len);
				out.arguments_.push_back(arg);
				pos += padded_size(str_len);
				if (pos > size) return false;
				break;
			}

			case 'b': {
				if (pos + 4 > size) return false;
				uint32_t len_be;
				std::memcpy(&len_be, data + pos, 4);
				size_t blob_len = swap_bytes(len_be);
				pos += 4;
				if (pos + blob_len > size) return false;

				Argument arg{out.arg_data_.size(), blob_len};
				out.arg_data_.insert(out.arg_data_.end(), data + pos, data + pos + blob_len);
				out.arguments_.push_back(arg);
				pos += ((blob_len + 3) / 4) * 4;
				if (pos > size) return false;
				break;
			}

			default:
				return false; // unsupported/unknown type tag
		}
	}

	return true;
}

int32_t OscMessage::arg_int32(size_t index) const {
	if (index >= arguments_.size() || type_tags_[index + 1] != 'i') return 0;
	uint32_t be;
	std::memcpy(&be, arg_data_.data() + arguments_[index].offset, 4);
	return static_cast<int32_t>(swap_bytes(be));
}

float OscMessage::arg_float32(size_t index) const {
	if (index >= arguments_.size() || type_tags_[index + 1] != 'f') return 0.0f;
	uint32_t be;
	std::memcpy(&be, arg_data_.data() + arguments_[index].offset, 4);
	uint32_t host_bits = swap_bytes(be);
	float value;
	std::memcpy(&value, &host_bits, 4);
	return value;
}

std::string OscMessage::arg_string(size_t index) const {
	if (index >= arguments_.size() || type_tags_[index + 1] != 's') return {};
	const Argument& arg = arguments_[index];
	return std::string(reinterpret_cast<const char*>(arg_data_.data() + arg.offset), arg.size);
}

void OscMessage::add_int32(int32_t value) {
	uint32_t be = swap_bytes(static_cast<uint32_t>(value));
	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&be);
	arguments_.push_back(Argument{arg_data_.size(), 4});
	arg_data_.insert(arg_data_.end(), bytes, bytes + 4);
	type_tags_ += 'i';
}

void OscMessage::add_float32(float value) {
	uint32_t host_bits;
	std::memcpy(&host_bits, &value, 4);
	uint32_t be = swap_bytes(host_bits);
	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&be);
	arguments_.push_back(Argument{arg_data_.size(), 4});
	arg_data_.insert(arg_data_.end(), bytes, bytes + 4);
	type_tags_ += 'f';
}

void OscMessage::add_string(const std::string& value) {
	arguments_.push_back(Argument{arg_data_.size(), value.size()});
	arg_data_.insert(arg_data_.end(), value.begin(), value.end());
	type_tags_ += 's';
}

void OscMessage::add_blob(const uint8_t* data, size_t size) {
	arguments_.push_back(Argument{arg_data_.size(), size});
	arg_data_.insert(arg_data_.end(), data, data + size);
	type_tags_ += 'b';
}

std::vector<uint8_t> OscMessage::serialize() const {
	std::vector<uint8_t> out;

	append_padded(out, address_.data(), address_.size());
	append_padded(out, type_tags_.data(), type_tags_.size());

	for (size_t i = 0; i < arguments_.size(); ++i) {
		const Argument& arg = arguments_[i];
		const uint8_t* content = arg_data_.data() + arg.offset;

		switch (type_tags_[i + 1]) {
			case 'i':
			case 'f':
				out.insert(out.end(), content, content + 4);
				break;

			case 's':
				append_padded(out, reinterpret_cast<const char*>(content), arg.size);
				break;

			case 'b': {
				uint32_t len_be = swap_bytes(static_cast<uint32_t>(arg.size));
				const uint8_t* len_bytes = reinterpret_cast<const uint8_t*>(&len_be);
				out.insert(out.end(), len_bytes, len_bytes + 4);
				out.insert(out.end(), content, content + arg.size);
				out.resize(out.size() + (((arg.size + 3) / 4) * 4 - arg.size), 0);
				break;
			}
		}
	}

	return out;
}
