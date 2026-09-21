#pragma once
#include "EASTL/string.h"

typedef struct Texture {
    eastl::string name;
    uint8_t  pmode;          // 0=4bpp, 1=8bpp, 2=16bpp, 3=24bpp
    bool     has_clut;
    uint16_t cx, cy;         // CLUT VRAM pos
    uint16_t cw, ch;         // CLUT size in 16-bit units
    uint16_t ix, iy;         // image VRAM pos
    uint16_t iw, ih;         // image size in 16-bit VRAM units (not pixels)
    uint16_t tpage;          // packed once after parse
    uint16_t clut_index;     // packed once after parse
    const uint16_t *pixels;  // points at image payload
    const uint16_t *clut;    // points at CLUT payload (or nullptr)
    uint32_t pixel_bytes;
    uint32_t clut_bytes;
} Texture;

void parseTexture(uint8_t *data, Texture *texture);