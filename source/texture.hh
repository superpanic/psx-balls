#pragma once
#include "psyqo/primitives/common.hh"
#include "psyqo/vector.hh"
#include "psyqo/fixed-point.hh"
#include <EASTL/string.h>

#define TIM_HEADER_SIZE (16)
#define TIM_MAGIC (0x10)
#define TIM_CLUT_FLAG (0x8)
#define TIM_IMAGE_FLAG (0x0)
#define TIM_HEADER_MAGIC_OFFSET (0)
#define TIM_HEADER_FLAG_OFFSET (4)
#define TIM_HEADER_CLUT_OFFSET (8)

typedef struct Texture {
	bool is_valid = false;
	eastl::string name;
	uint8_t pmode;          // 0=4bpp, 1=8bpp, 2=16bpp, 3=24bpp
	bool has_clut;
	int16_t cx, cy;         // CLUT VRAM pos
	int16_t cw, ch;         // CLUT size in 16-bit units
	int16_t ix, iy;         // image VRAM pos
	int16_t iw, ih;         // image size in 16-bit VRAM units (not pixels)
	uint16_t tpage;          // packed once after parse
	uint16_t clut_index;     // packed once after parse
	const uint16_t *pixels;  // points at image payload
	const uint16_t *clut;    // points at CLUT payload (or nullptr)
	uint32_t pixel_bytes;
	uint32_t clut_bytes;
	bool isValid() const { return is_valid; }
} Texture;

void parse_TIM(uint8_t *data, size_t size, Texture *texture);
uint16_t pack_TPage(uint16_t x, uint16_t y, uint16_t mode);
uint16_t pack_CLUT(uint16_t x, uint16_t y);
auto toUVCoords(psyqo::FixedPoint<> ufix, psyqo::FixedPoint<> vfix, Texture &tex);
