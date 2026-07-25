#include "gbl_parser.hh"
#include "psyqo/xprintf.h"
#include "parse_macros.hh"

bool parse_GBL(const uint8_t *data, size_t size, Mesh *mesh) {

    if(size < sizeof(uint32_t)) {
        return false; // Not enough data for magic number
    }

    // big endian byte copy
    uint32_t magic = READ_BE32(data);
    
    if(magic != GLTF_MAGIC) {
        return false; // Invalid magic number
    } else {
        printf("GLTF magic number verified: 0x%08X\n", magic);
    }

/*
    // Read header
    uint32_t numVertices = *(const uint32_t *)(data);
    uint32_t numIndices = *(const uint32_t *)(data + 4);
    uint32_t vertexDataOffset = *(const uint32_t *)(data + 8);

    if(size < vertexDataOffset + numVertices * sizeof(psyqo::Vec3) + numIndices * sizeof(uint8_t)) {
        return false; // Not enough data for vertices and indices
    }

    if(numVertices > SMALL_MODEL_MAX_VERTICES || numIndices > SMALL_MODEL_MAX_INDICES) {
        return false; // Exceeds maximum allowed vertices or indices
    }

    mesh->numVertices = numVertices;
    mesh->numIndices = numIndices;
    mesh->vertices = (psyqo::Vec3 *)(data + vertexDataOffset);
    mesh->indices = (uint8_t *)(data + vertexDataOffset + numVertices * sizeof(psyqo::Vec3));
*/

    return true;

}