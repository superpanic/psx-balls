#include "level_buffer.hh"
#include "psyqo/xprintf.h"

#include <cstddef>
#include <cstdint>

namespace level_buffer {

constexpr size_t BUFFER_SIZE = 512*1024;
alignas(4) static uint8_t g_buffer[BUFFER_SIZE];
static uint8_t *g_buffer_end = g_buffer;

uint8_t *buffer_alloc(size_t n) {
	n = (n + 3) & ~3; // align
	if (g_buffer_end + n > g_buffer + sizeof(g_buffer)) return nullptr;
	uint8_t *p = g_buffer_end;
	g_buffer_end += n;
	return p;
}

void buffer_reset() { 
	g_buffer_end = g_buffer; 
}

size_t buffer_free() {
    return BUFFER_SIZE - (g_buffer_end - g_buffer);
}

} // namespace level_buffer
