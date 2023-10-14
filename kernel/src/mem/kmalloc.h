#pragma once

#include <stddef.h>

size_t kmalloc_init();

// quick and dirty memory allocator,
// returns a pointer to an area of free memory
// at least _len_ bytes long.
void *kmalloc(size_t len);

void kfree(void *ptr);

size_t get_free_pages();
