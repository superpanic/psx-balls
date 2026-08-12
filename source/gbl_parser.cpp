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

	Accessor accessors[MAX_BUFFERVIEWS];
	BufferView bufferViews[MAX_BUFFERVIEWS];

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

	uint32_t json_chunk_length = READ_LE32(head); head += 4;
	uint32_t json_chunk_type = READ_LE32(head); head += 4;
	if(json_chunk_type != 0x4E4F534A) { // "JSON"
		printf("Expected JSON chunk, got type: 0x%08X\n", json_chunk_type);
		return false; // not a JSON chunk
	} else {
		printf("JSON chunk length: %u\n", json_chunk_length);
	}

	// get  meshes  array JSON pointer
	head = (const uint8_t*)find_key((const char*)head, "meshes");
	psyqo::Kernel::assert(head != nullptr, "meshes ARRAY not found in glTF JSON chunk");
	meshes_ptr = head;

	// get  accessors  array JSON pointer
	head = (const uint8_t*)find_key((const char*)head, "accessors");
	psyqo::Kernel::assert(head != nullptr, "accessors ARRAY not found in glTF JSON chunk");
	accessors_ptr = head;

	// get  bufferViews  array JSON pointer
	head = (const uint8_t*)find_key((const char*)head, "bufferViews");
	psyqo::Kernel::assert(head != nullptr, "bufferViews ARRAY not found in glTF JSON chunk");
	bufferViews_ptr = head;

	// get  buffers  array JSON pointer
	head = (const uint8_t*)find_key((const char*)head, "buffers");
	psyqo::Kernel::assert(head != nullptr, "buffers ARRAY not found in glTF JSON chunk");
	buffers_ptr = head;

	// get  BINARY  chunk pointer
	head = data + GLTF_JSON_OFFSET + json_chunk_length; // binary chunk starts after the JSON chunk
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

	// jump to accessors array to parse the first accessor
	head = accessors_ptr;
	int32_t t;
	short bufferView_count = 0;

	for(int i=0; i<MAX_BUFFERVIEWS; i++) {
		accessors[i] = {};
		
		// read bufferView
		head = (const uint8_t*)find_key((const char*)head, "bufferView");
		if(!read_long(head, t)) {
			psyqo::Kernel::assert(i >= 1, "Failed to find any bufferViews in glTF JSON chunk");
			break;
		}
		accessors[i].bufferView = (uint32_t)t;

		// read componentType
		head = (const uint8_t*)find_key((const char*)head, "componentType");
		psyqo::Kernel::assert(read_long(head, t), "Failed to read componentType in glTF JSON chunk");
		accessors[i].componentType = (uint32_t)t;
		psyqo::Kernel::assert(isValidComponentType(accessors[i].componentType), "invalid componentType in glTF JSON chunk");

		// read count
		head = (const uint8_t*)find_key((const char*)head, "count");
		psyqo::Kernel::assert(read_long(head, t), "Failed to read count in glTF JSON chunk");
		accessors[i].count = (uint32_t)t;

		// read type
		head = (const uint8_t*)find_key((const char*)head, "type");
		read_string((const char*&)head, accessors[i].type);
		psyqo::Kernel::assert(accessors[i].type.length() > 0, "Failed to read type in glTF JSON chunk");

		bufferView_count++;
		printf("accessor[%d]: bufferView=%u, componentType=%u, count=%u, type=%s\n", i, accessors[i].bufferView, accessors[i].componentType, accessors[i].count, accessors[i].type.c_str());
	}
		
	// jump to bufferViews array to parse the first bufferView
	head = bufferViews_ptr;
	
	for(int i=0; i<bufferView_count; i++) {
		bufferViews[i] = {};
	
		// read buffer
		head = (const uint8_t*)find_key((const char*)head, "buffer");
		if(!read_long(head, t)) {
			psyqo::Kernel::assert(i >= 1, "Failed to find any bufferViews in glTF JSON chunk");
			break;
		}
		bufferViews[i].buffer = (uint32_t)t;

		// read byteLength
		head = (const uint8_t*)find_key((const char*)head, "byteLength");
		psyqo::Kernel::assert(read_long(head, t), "Failed to read byteLength in glTF JSON chunk");
		bufferViews[i].byteLength = (uint32_t)t;

		// read byteOffset
		head = (const uint8_t*)find_key((const char*)head, "byteOffset");
		psyqo::Kernel::assert(read_long(head, t), "Failed to read byteOffset in glTF JSON chunk");
		bufferViews[i].byteOffset = (uint32_t)t;

		// calculate byteStride
		bufferViews[i].byteStride = bufferViews[i].byteLength / accessors[i].count;

		printf("bufferView[%d]: buffer=%u, byteOffset=%u, byteLength=%u, byteStride=%u\n", i, bufferViews[i].buffer, bufferViews[i].byteOffset, bufferViews[i].byteLength, bufferViews[i].byteStride);
	}

	head = bin_ptr + bufferViews[0].byteOffset;
	psyqo::Kernel::assert(head + bufferViews[0].byteLength <= end, "BufferView exceeds binary chunk size");

	// read vertices
	for(int i=0; i<accessors[0].count && i<SMALL_MODEL_MAX_VERTICES; i++) {
		psyqo::Vec3 &v = mesh->vertices[i];
		if (accessors[0].componentType == FLOAT && accessors[0].type == "VEC3") {
        	uint32_t bx = READ_LE32(head); head += 4;
        	uint32_t by = READ_LE32(head); head += 4;
        	uint32_t bz = READ_LE32(head); head += 4;
			v.x = psyqo::FixedPoint<>(float32_bits_to_fixed12(bx), psyqo::FixedPoint<>::RAW);
			v.y = psyqo::FixedPoint<>(float32_bits_to_fixed12(by), psyqo::FixedPoint<>::RAW);
			v.z = psyqo::FixedPoint<>(float32_bits_to_fixed12(bz), psyqo::FixedPoint<>::RAW);
			printf("vertex[%d]: x=%d, y=%d, z=%d\n", i, v.x.raw(), v.y.raw(), v.z.raw());
		} else {
			psyqo::Kernel::assert(false, "Unsupported accessor componentType or type for vertices");
		}
	}
	mesh->num_vertices = accessors[0].count;

	// read indices
	head = bin_ptr + bufferViews[3].byteOffset;
	psyqo::Kernel::assert(head + bufferViews[3].byteLength <= end, "BufferView exceeds binary chunk size");
	for(int i=0; i<accessors[3].count && i<SMALL_MODEL_MAX_INDICES; i++) {
		if (accessors[3].componentType == UNSIGNED_SHORT && accessors[3].type == "SCALAR") {
			uint16_t index = READ_LE16(head); head += 2;
			mesh->indices[i] = index;
			printf("index[%d]: %u\n", i, index);
		} else {
			psyqo::Kernel::assert(false, "Unsupported accessor componentType or type for indices");
		}
	}
	mesh->num_indices = accessors[3].count;

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


// Convert little-endian IEEE-754 binary32 bits → FixedPoint raw value (scale 4096)
static int32_t float32_bits_to_fixed12(uint32_t bits) {
	const uint32_t sign     = bits >> 31;
	const int32_t  exponent = int32_t((bits >> 23) & 0xFF) - 127; // unbiased
	uint32_t       mantissa = (bits & 0x7FFFFF);

	// Zero / denormal → treat as 0 (good enough for vertex data)
	if (exponent == -127) {
		return 0;
	}

	// Inf / NaN → clamp (should never appear in a well-formed glTF)
	if (exponent == 128) {
		return sign ? 0x80000000 : 0x7FFFFFFF;
	}

	// Add the implicit leading 1
	mantissa |= 0x800000;

	// We want: value * 4096 = mantissa * 2^(exponent-23) * 2^12
	// → shift = exponent - 23 + 12 = exponent - 11
	const int32_t shift = exponent - 11;

	int32_t value;
	if (shift >= 0) {
		// Make sure we don't shift into oblivion (vertex data is normally small)
		if (shift > 8) {          // safety for huge numbers
			value = 0x7FFFFFFF;
		} else {
			value = int32_t(mantissa << shift);
		}
	} else {
		value = int32_t(mantissa >> -shift);
		// Optional: round to nearest
		// if ((mantissa >> (-shift-1)) & 1) value += 1;
	}

	return sign ? -value : value;
}

// reads a string from the JSON, advancing the pointer past it. The string is expected to be enclosed in double quotes.
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
bool read_long(const uint8_t *&p, int32_t &out) {
	const uint8_t* start = p;
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

size_t get_accessor_size_from_string(const char *type) {
	static const size_t str_size = 8;
	if (stringncompare(type, "SCALAR", str_size) == 0) return 1;
	if (stringncompare(type, "VEC2", str_size) == 0) return 2;
	if (stringncompare(type, "VEC3", str_size) == 0) return 3;
	if (stringncompare(type, "VEC4", str_size) == 0) return 4;
	if (stringncompare(type, "MAT2", str_size) == 0) return 4;
	if (stringncompare(type, "MAT3", str_size) == 0) return 9;
	if (stringncompare(type, "MAT4", str_size) == 0) return 16;
	return 0; // unknown type
}

bool isValidComponentType(uint32_t componentType) {
	for (uint32_t type : componentTypes) {
		if (type == componentType) {
			return true;
		}
	}
	return false;
}
