#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

// use 4K pages unless you have a very good reason not to
#define PAGE_SIZE 0x1000

#define HIGHER_HALF 0xFFFF800000000000

void pmm_init(struct limine_memmap_response* memmap);

// allocates <pages> pages of physical memory
void* pmm_alloc(size_t pages);
// frees <count> pages of physical memory from address <ptr>
void pmm_free(void* ptr, size_t count);
