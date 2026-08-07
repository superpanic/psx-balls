#pragma once

#include "psyqo/vector.hh"
#include "EASTL/string.h"

#define GLTF_MAGIC 0x676C5446  // "glTF"

#define SMALL_MODEL_MAX_VERTICES (256)
#define SMALL_MODEL_MAX_INDICES (256)

typedef struct Mesh {
	eastl::string name;
	psyqo::Vec3 vertices[SMALL_MODEL_MAX_VERTICES];
	uint8_t indices[SMALL_MODEL_MAX_INDICES];
	unsigned num_vertices = 0;
	unsigned num_indices = 0;
	bool isValid() const { return num_vertices > 0 && num_indices > 0; }
} Mesh;

typedef struct Object {
	Mesh *mesh;
	psyqo::Vec3 position;
	psyqo::Vec3 rotation;
	psyqo::Vec3 scale;
} Object;

// main parser
bool parse_GBL(const uint8_t *data, size_t size, Mesh *mesh);

// helpers
const char* find_key(const char* json, const char* key);
bool parse_fixed(const char *&p, psyqo::FixedPoint<> &out);
const char* skip_value(const char* p);
const char* skip_whitespace(const char* p);
bool read_long(const char*& p, int32_t& out);
void read_string(const char *&p, eastl::string &out);