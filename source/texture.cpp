#include "texture.hh"
#include "psyqo/xprintf.h"
#include "parser_macros.hh"

void parse_TIM(uint8_t *data, size_t size, Texture *texture) {
	const uint8_t *head = data;
	if (size < TIM_HEADER_SIZE) {
		texture->is_valid = false;
		printf("ERROR: TIM data is too small\n");
		return;
	}

	if (head[TIM_HEADER_MAGIC_OFFSET] != TIM_MAGIC) {
		texture->is_valid = false;
		printf("ERROR: TIM magic is incorrect\n");
		return;
	}

	if (head[TIM_HEADER_FLAG_OFFSET] & TIM_CLUT_FLAG) {
		// CLUT is present
		texture->has_clut = true;
	} else {
		// No CLUT
		texture->has_clut = false;
	}

	texture->pmode = READ_LE8(head + TIM_HEADER_FLAG_OFFSET) & 0x3;
	uint8_t pmode;
	switch (texture->pmode) {
		case 0:
			pmode = 4;
			// 4bpp
			break;
		case 1:
			pmode = 8;
			// 8bpp
			break;
		case 2:
			pmode = 16;
			// 16bpp
			break;
		case 3:
			pmode = 24;
			// 24bpp
			break;
		default:
			printf("ERROR: Unknown pixel mode\n");
			texture->is_valid = false;
			return;
	}
	printf("Pixel mode: %u\n", pmode);
	printf("Has CLUT: %s\n", texture->has_clut ? "true" : "false");

	uint8_t *clut_data = (uint8_t *)(head + TIM_HEADER_CLUT_OFFSET);
	uint8_t *image_data = clut_data;
	if(texture->has_clut) {
		uint32_t clut_size = READ_LE32(clut_data); // 4 bytes for CLUT size, cast to uint32_t
		image_data = clut_data + clut_size;
		clut_data += 4; // Move past the CLUT size field
		printf("CLUT size: %u\n", clut_size);
		texture->cx = READ_LE16(clut_data);
		clut_data += 2;
		texture->cy = READ_LE16(clut_data);
		clut_data += 2;
		texture->cw = READ_LE16(clut_data);
		clut_data += 2;
		texture->ch = READ_LE16(clut_data);
		clut_data += 2;
		texture->clut = (const uint16_t *)clut_data;
	} else {
		texture->clut = nullptr;
	}

	printf("CLUT X: %u\n", texture->cx);
	printf("CLUT Y: %u\n", texture->cy);
	printf("CLUT W: %u\n", texture->cw);
	printf("CLUT H: %u\n", texture->ch);

	uint32_t image_size = READ_LE32(image_data); // 4 bytes for image size
	image_data += 4; // Move past the image size field
	texture->ix = READ_LE16(image_data);
	image_data += 2;
	texture->iy = READ_LE16(image_data);
	image_data += 2;
	texture->iw = READ_LE16(image_data);
	image_data += 2;
	texture->ih = READ_LE16(image_data);
	image_data += 2;
	texture->pixels = (const uint16_t *)image_data;

	printf("Image X: %u\n", texture->ix);
	printf("Image Y: %u\n", texture->iy);
	printf("Image W: %u\n", texture->iw);
	printf("Image H: %u\n", texture->ih);
	printf("Image size: %u\n", image_size);

	texture->is_valid = true;
}

uint16_t pack_TPage(uint16_t x, uint16_t y, uint16_t mode) {
	uint16_t pageX = x/64;
	uint16_t pageY = y/256;
	return (pageX & 0xF) | ((pageY & 1) << 4) | ((mode & 0x3) << 7);
}

uint16_t pack_CLUT(uint16_t x, uint16_t y) {
	return ((y & 0x1FF) << 6) | ((x >> 4) & 0x3F);
}

psyqo::PrimPieces::UVCoords toUVCoords(psyqo::FixedPoint<> ufix, psyqo::FixedPoint<> vfix, Texture &tex) {
	int tex_width = tex.iw * 4; // assuming 4bpp
	int tex_height = tex.ih;
	int u = (ufix.raw() * tex_width) >> 12;
	int v = (vfix.raw() * tex_height) >> 12;
	// glTF V=0 is bottom; PS1 V=0 is top
//	v = tex_height - 1 - v;
	// clamp 0...255
	if (u < 0) u = 0; if (u > 255) u = 255;
	if (v < 0) v = 0; if (v > 255) v = 255;
	return psyqo::PrimPieces::UVCoords{ uint8_t(u), uint8_t(v) };
}