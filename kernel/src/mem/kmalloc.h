#pragma once

#include <stddef.h>

void kmalloc_init(void* start, size_t len);

// quick and dirty memory allocator,
// returns a pointer to an area of free memory
// at least _len_ bytes long.
void *kmalloc(size_t len);

void kfree(void *ptr);