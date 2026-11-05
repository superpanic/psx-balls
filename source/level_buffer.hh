#pragma once

namespace level_buffer {

alignas(4) static uint8_t g_buffer[512*1024];
static uint8_t *g_buffer_end = g_buffer;

uint8_t *buffer_alloc(size_t n) {
    n = (n + 3) & ~3; // align
    if (g_buffer_end + n > g_buffer + sizeof(g_buffer)) return nullptr;
    uint8_t *p = g_buffer_end;
    g_buffer_end += n;
    return p;
}

void buffer_reset() { g_buffer_end = g_buffer; }

} // namespace level_buffer
