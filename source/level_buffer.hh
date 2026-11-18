#pragma once

#include <cstdint>
#include <cstddef>

namespace level_buffer {

uint8_t *buffer_alloc(size_t n);
void buffer_reset();
size_t buffer_bytes_free();
size_t total_bytes();

} // namespace level_buffer
