#pragma once

#include "psyqo/vector.hh"

typedef struct Mesh {
    psyqo::Vec3 *vertices;
    uint8_t *indices;
    unsigned numVertices;
    unsigned numIndices;
} Mesh;

typedef struct Object {
    Mesh *mesh;
    psyqo::Vec3 position;
    psyqo::Vec3 rotation;
    psyqo::Vec3 scale;
} Object;
