#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

// use 4K pages unless you have a very good reason not to
#define PAGE_SIZE 0x1000

#define HIGHER_HALF 0xFFFF800000000000

// mmap flags
#define MMAP_PROT_NONE   0x00
#define MMAP_PROT_READ   0x01
#define MMAP_PROT_WRITE  0x02
#define MMAP_PROT_EXEC   0x04
#define MMAP_MAP_PRIVATE 0x01
#define MMAP_MAP_SHARED  0x02
#define MMAP_MAP_FIXED   0x04
#define MMAP_MAP_ANON    0x08

void pmm_init(struct limine_memmap_response* memmap);

// allocates <pages> pages of physical memory
void* pmm_alloc(size_t pages);
// frees <count> pages of physical memory from address <ptr>
void pmm_free(void* ptr, size_t count);
