#include "gbl_parser.hh"
#include "psyqo/xprintf.h"
#include "psyqo/kernel.hh"
#include "parser_macros.hh"
#include "str_tools.hh"
#include "psyqo/fixed-point.hh"

bool parse_GBL(const uint8_t *data, size_t size, Mesh *mesh) {
	const uint8_t *head = data;
	const uint8_t *meshes_ptr = nullptr;
	const uint8_t *accessors_ptr = nullptr;
	const uint8_t *bufferViews_ptr = nullptr;
	const uint8_t *buffers_ptr = nullptr;
	const uint8_t *bin_ptr = nullptr;
	const uint8_t *end = data + size;

	if(size < sizeof(uint32_t)) {
		return false; // Not enough data for magic number
	}

	// big endian 4 byte magic number copy
	uint32_t magic = READ_BE32(head);
	head += 4;
	if(magic != GLTF_MAGIC) {
		return false; // Invalid magic number
	} else {
		printf("GLTF magic number verified: 0x%08X\n", magic);
	}

	uint32_t version = READ_LE32(head); head += 4;
	if(version != 2) {
		printf("Unsupported GLTF version: %u\n", version);
		return false; // Unsupported version
	} else {
		printf("GLTF version: %u\n", version);
	}

	uint32_t length = READ_LE32(head); head += 4;
	if(length != size) {
		printf("GLTF length mismatch: expected %u, got %zu\n", length, size);
		return false; // Length mismatch
	} else {
		printf("GLTF length: %u\n", length);
	}

	uint32_t chunk_length = READ_LE32(head); head += 4;
	uint32_t chunk_type = READ_LE32(head); head += 4;
	if(chunk_type != 0x4E4F534A) { // "JSON"
		printf("Expected JSON chunk, got type: 0x%08X\n", chunk_type);
		return false; // not a JSON chunk
	} else {
		printf("JSON chunk length: %u\n", chunk_length);
	}

	// find JSON keys for meshes, accessors, bufferViews, buffers
	head = (const uint8_t*)find_key((const char*)head, "meshes");
	psyqo::Kernel::assert(head != nullptr, "meshes ARRAY not found in glTF JSON chunk");
	meshes_ptr = head;

	head = (const uint8_t*)find_key((const char*)head, "accessors");
	psyqo::Kernel::assert(head != nullptr, "accessors ARRAY not found in glTF JSON chunk");
	accessors_ptr = head;

	head = (const uint8_t*)find_key((const char*)head, "bufferViews");
	psyqo::Kernel::assert(head != nullptr, "bufferViews ARRAY not found in glTF JSON chunk");
	bufferViews_ptr = head;

	head = (const uint8_t*)find_key((const char*)head, "buffers");
	psyqo::Kernel::assert(head != nullptr, "buffers ARRAY not found in glTF JSON chunk");
	buffers_ptr = head;

	// get binary chunk pointer
	head = data + GLTF_JSON_OFFSET + chunk_length; // binary chunk starts after the JSON chunk
	uint32_t bin_chunk_length = READ_LE32(head); head += 4;
	uint32_t bin_chunk_type = READ_LE32(head); head += 4;
	if(bin_chunk_type != BIN_MAGIC) {
		printf("Expected BIN chunk, got type: 0x%08X\n", bin_chunk_type);
		return false; // not a BIN chunk
	} else {
		printf("BIN chunk length: %u\n", bin_chunk_length);
		bin_ptr = head;
	}

	// jump back to meshes array to parse the first mesh
	head = meshes_ptr;

	// mesh name
	head = (const uint8_t*)find_key((const char*)head, "name");
	psyqo::Kernel::assert(head != nullptr, "mesh NAME not found in glTF JSON chunk");
	read_string((const char*&)head, mesh->name);
	printf("mesh name: %s\n", mesh->name.c_str());

	short attr_position, attr_normal, attr_texcoord_0, attr_indices;
	int32_t temp_val = 0;

	// parse POSITION attribute
	head = (const uint8_t*)find_key((const char*)head, "POSITION");
	psyqo::Kernel::assert(head != nullptr, "POSITION attribute not found in glTF JSON chunk");
	psyqo::Kernel::assert(read_long((const char*&)head, temp_val), "Failed to read POSITION attribute index");
	attr_position = static_cast<short>(temp_val);

	// parse NORMAL attribute
	head = (const uint8_t*)find_key((const char*)head, "NORMAL");
	psyqo::Kernel::assert(head != nullptr, "NORMAL attribute not found in glTF JSON chunk");
	psyqo::Kernel::assert(read_long((const char*&)head, temp_val), "Failed to read NORMAL attribute index");
	attr_normal = static_cast<short>(temp_val);

	// parse TEXCOORD_0 attribute
	head = (const uint8_t*)find_key((const char*)head, "TEXCOORD_0");
	psyqo::Kernel::assert(head != nullptr, "TEXCOORD_0 attribute not found in glTF JSON chunk");
	psyqo::Kernel::assert(read_long((const char*&)head, temp_val), "Failed to read TEXCOORD_0 attribute index");
	attr_texcoord_0 = static_cast<short>(temp_val);

	// parse INDICES attribute
	head = (const uint8_t*)find_key((const char*)head, "indices");
	psyqo::Kernel::assert(head != nullptr, "INDICES attribute not found in glTF JSON chunk");
	psyqo::Kernel::assert(read_long((const char*&)head, temp_val), "Failed to read INDICES attribute index");
	attr_indices = static_cast<short>(temp_val);

	printf("Parsed attributes: POSITION=%d, NORMAL=%d, TEXCOORD_0=%d, INDICES=%d\n",
		attr_position, attr_normal, attr_texcoord_0, attr_indices);

		

	return true;
}

// returns a pointer to the character after the key, or nullptr if not found.
const char *find_key(const char *json, const char *key) {
	const char *found = stringstring(json, key);
	if(!found) {
		return nullptr; // Key not found
	}

	// Move pointer to the character after the key
	found += stringlength(key);

	// Skip whitespace and colon
	while(*found && (*found == ' ' || *found == '\t' || *found == '\n' || *found == '\r' || *found == ':')) {
		found++;
	}

	return found;
}

/**
 * parse a decimal number from a string into a FixedPoint.
 * supports: 123, -123, 123.456, -0.5, .75, etc.
 * stops at the first non-numeric character.
 *
 * returns true on success, false on empty / invalid input.
 * the pointer `p` is advanced past the number.
 */
bool parse_fixed(const char *&p, psyqo::FixedPoint<> &out) {
	if (!p || !*p) return false;

	// skip leading whitespace
	while (*p == ' ' || *p == '\t') ++p;

	// handle sign
	bool negative = false;
	if (*p == '-') {
		negative = true;
		++p;
	} else if (*p == '+') {
		++p;
	}

	// read integer part
	int32_t integer = 0;
	bool hasDigits = false;
	while (*p >= '0' && *p <= '9') {
		hasDigits = true;
		integer = integer * 10 + (*p - '0');
		++p;
	}

	// read fractional part
	int32_t fraction = 0;
	int32_t divisor = 1;

	if (*p == '.') {
		++p;
		// We only need enough precision for 12 fractional bits (~3-4 decimal digits)
		// Reading more is fine; we'll scale it down later.
		int digits = 0;
		while (*p >= '0' && *p <= '9' && digits < 6) { // limit to avoid overflow
			hasDigits = true;
			fraction = fraction * 10 + (*p - '0');
			divisor *= 10;
			++p;
			++digits;
		}
		// Skip any remaining fractional digits we don't care about
		while (*p >= '0' && *p <= '9') ++p;
	}

	// no digits found at all
	if (!hasDigits) return false;

	// convert fractional part to fixed-point scale (4096)
	// fraction / divisor * 4096
	int32_t fixedFrac = 0;
	if (divisor > 1) {
		// 64-bit intermediate to stay safe
		fixedFrac = static_cast<int32_t>(
			(static_cast<int64_t>(fraction) * psyqo::FixedPoint<>::scale) / divisor
		);
	}

	int32_t raw = integer * psyqo::FixedPoint<>::scale + fixedFrac;
	if (negative) raw = -raw;

	out = psyqo::FixedPoint<>(raw, psyqo::FixedPoint<>::RAW);
	return true;
}

void read_string(const char *&p, eastl::string &out) {

	if (!p || *p != '"') return; // Not a string
	++p; // skip title quote
	if(*p == ':') {
		++p; // skip colon if present
		if(*p == '"') ++p; // skip opening quote if present
	}
	const char *start = p;
	while (*p && *p != '"') {
		if (*p == '\\') ++p; // skip escaped character
		++p;
	}
	out.assign(start, p - start);
	if (*p == '"') ++p; // skip closing quote
}

// skip a full value (object, array, string, number...)
const char* skip_value(const char *p) {
	if (!p) return nullptr;

	// Skip whitespace
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;

	if (*p == '{') {
		// Skip object
		int depth = 1;
		++p;
		while (depth > 0 && *p) {
			if (*p == '{') ++depth;
			else if (*p == '}') --depth;
			++p;
		}
	} else if (*p == '[') {
		// Skip array
		int depth = 1;
		++p;
		while (depth > 0 && *p) {
			if (*p == '[') ++depth;
			else if (*p == ']') --depth;
			++p;
		}
	} else if (*p == '"') {
		// Skip string
		++p; // skip opening quote
		while (*p && *p != '"') {
			if (*p == '\\') ++p; // skip escaped character
			++p;
		}
		if (*p == '"') ++p; // skip closing quote
	} else {
		// skip number or literal (true, false, null)
		while (*p && *p != ',' && *p != '}' && *p != ']' && !isspace(*p)) ++p;
	}

	return p;
}

const char* skip_whitespace(const char *p) {
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
	return p;
}

// moves the pointer forward and returns true if a number was read, false otherwise
bool read_long(const char *&p, int32_t& out) {
	const char* start = p;
	bool negative = false;

	while(*p == ' ' || *p == '\t' || *p == '"' || *p == ':') ++p; // skip whitespace and quotes and colons

	if (*p == '-') {
		negative = true;
		++p;
	} else if (*p == '+') {
		++p;
	}

	if (!isnumber(*p)) {
		p = start; // reset pointer on failure
		return false;
	}

	int32_t value = 0;
	while (isnumber(*p)) {
		value = value * 10 + (*p - '0');
		++p;
	}

	out = negative ? -value : value;
	return true;
}