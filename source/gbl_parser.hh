#pragma once

#include "psyqo/vector.hh"

#define GBL_MAGIC 0x47424C00   // "GBL\0"
#define GBL_VERSION (1)
#define GBL_HEADER_SIZE (12)
#define SMALL_MODEL_MAX_VERTICES (256)
#define SMALL_MODEL_MAX_INDICES (256)

typedef struct Mesh {
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

bool parse_GBL(const uint8_t *data, size_t size, Mesh *mesh);