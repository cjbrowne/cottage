#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    void* top_level;
    void** mmap_ranges;
} pagemap_t;

bool map_page(pagemap_t* pagemap, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);
