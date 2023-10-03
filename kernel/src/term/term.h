// kernel-level terminal emulator abstraction layer
#pragma once

#include <stddef.h>
#include <stdint.h>

#define TERM_WRITE_BUF(buf) term_write(buf, sizeof(buf))

void term_init(uint32_t *framebuffer, size_t width, size_t height, size_t pitch);
void term_write(const char* string, size_t count);