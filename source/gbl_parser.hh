#pragma once

#include "psyqo/vector.hh"
#include "EASTL/string.h"
#include "str_tools.hh"
#include "texture.hh"

#define GLTF_MAGIC 0x676C5446  // "glTF"
#define BIN_MAGIC 0x004E4942  // "BIN\0"
#define GLTF_JSON_OFFSET (20)

#define SMALL_MODEL_MAX_VERTICES (256)
#define SMALL_MODEL_MAX_INDICES (256)
#define MAX_BUFFERVIEWS (8)

#define SCALAR_SIZE  (1)
#define VEC2_SIZE    (2)
#define VEC3_SIZE    (3)
#define VEC4_SIZE    (4)
#define MAT2_SIZE    (4)
#define MAT3_SIZE    (9)
#define MAT4_SIZE   (16)

#define SIGNED_BYTE    (5120)
#define UNSIGNED_BYTE (5121)
#define SIGNED_SHORT   (5122)
#define UNSIGNED_SHORT (5123)
#define SIGNED_INT     (5124)
#define UNSIGNED_INT (5125)
#define FLOAT   (5126)

const static uint32_t componentTypes[] = {
	SIGNED_BYTE,
	UNSIGNED_BYTE,
	SIGNED_SHORT,
	UNSIGNED_SHORT,
	SIGNED_INT,
	UNSIGNED_INT,
	FLOAT
};

typedef struct Mesh {
	eastl::string name;
	psyqo::Vec3 vertices[SMALL_MODEL_MAX_VERTICES];
	uint8_t indices[SMALL_MODEL_MAX_INDICES];
	psyqo::Vec2 texcoords[SMALL_MODEL_MAX_VERTICES];
	unsigned num_vertices = 0;
	unsigned num_texcoords = 0;
	unsigned num_indices = 0;
	bool isValid() const { return num_vertices > 0 && num_indices > 0; }
} Mesh;

typedef struct Object {
	Mesh *mesh;
	Texture *texture;
	psyqo::Vec3 position;
	psyqo::Vec3 rotation;
	psyqo::Vec3 scale;
} Object;

typedef struct Accessor {
	uint32_t bufferView;
	uint32_t componentType;
	uint32_t count;
	eastl::string type;
} Accessor;


typedef struct BufferView {
	uint32_t buffer;
	uint32_t byteOffset;
	uint32_t byteLength;
	uint32_t byteStride;
} BufferView;

// main parser
bool parse_GBL(const uint8_t *data, size_t size, Mesh *mesh);

// helpers
const char* find_key(const char* json, const char* key);
const char* skip_value(const char* p);
const char* skip_whitespace(const char* p);
bool read_long(const uint8_t*& p, int32_t &out);
void read_string(const char *&p, eastl::string &out);
size_t get_accessor_size_from_string(const char *type);
bool isValidComponentType(uint32_t componentType);
static int32_t float32_bits_to_fixed12(uint32_t bits);