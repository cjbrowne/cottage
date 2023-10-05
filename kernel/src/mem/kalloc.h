#pragma once

#include <stddef.h>

// quick and dirty memory allocator,
// returns a pointer to an area of free memory
// at least _len_ bytes long.
void *kalloc(size_t len);

void kfree(void *ptr);