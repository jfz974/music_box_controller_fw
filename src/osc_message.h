#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// A single OSC 1.0 message (address pattern + type-tagged arguments), not a
// bundle. Supports the four core argument types: int32 (i), float32 (f),
// string (s) and blob (b). See docs/osc/README.md for the wire format this
// class implements and docs/osc/*.md for the commands built on top of it.
class OscMessage {
public:
	OscMessage() = default;

	// Starts an empty outgoing message. Arguments are appended in order with
	// add_int32/add_float32/add_string/add_blob.
	explicit OscMessage(std::string address) : address_(std::move(address)) {}

	// Parses a single OSC message out of a raw byte buffer (already
	// excludes any bundle framing -- see docs/osc/README.md). On success,
	// fills out and returns true; on any malformed input returns false and
	// leaves out unspecified.
	static bool parse(const uint8_t* data, size_t size, OscMessage& out);

	const std::string& address() const { return address_; }

	// Type tag string including the leading ','. e.g. ",si" for a
	// (string, int32) argument pair; "," (no tags) for no arguments.
	const std::string& type_tags() const { return type_tags_; }

	size_t argument_count() const { return arguments_.size(); }

	// Argument readers, for a message obtained from parse(). index is
	// 0-based; a mismatched type or an out-of-range index returns a
	// default-constructed value.
	int32_t arg_int32(size_t index) const;
	float arg_float32(size_t index) const;
	std::string arg_string(size_t index) const;

	// Argument writers, for a message being built to send.
	void add_int32(int32_t value);
	void add_float32(float value);
	void add_string(const std::string& value);
	void add_blob(const uint8_t* data, size_t size);

	// Serializes address + type tag string + arguments into a 4-byte
	// aligned OSC byte buffer, ready to hand to tud_vendor_write.
	std::vector<uint8_t> serialize() const;

private:
	// Byte range of one argument's content within arg_data_: the 4 raw wire
	// bytes for i/f, or the raw (unpadded, unterminated) content for s/b.
	struct Argument {
		size_t offset;
		size_t size;
	};

	std::string address_;
	std::string type_tags_ = ",";
	std::vector<uint8_t> arg_data_;
	std::vector<Argument> arguments_;
};
