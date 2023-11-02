#pragma once

#include <resource/resource.h>
#include <mem/pagemap.h>

// flags
#define MMAP_PROT_NONE   0x00
#define MMAP_PROT_READ   0x01
#define MMAP_PROT_WRITE  0x02
#define MMAP_PROT_EXEC   0x04
#define MMAP_MAP_PRIVATE 0x01
#define MMAP_MAP_SHARED  0x02
#define MMAP_MAP_FIXED   0x04
#define MMAP_MAP_ANON    0x08

typedef struct mmap_range_local_s mmap_range_local_t;

typedef struct {
    pagemap_t shadow_pagemap;
    mmap_range_local_t** locals;
    size_t num_locals;
    resource_t* resource;
    uint64_t base;
    uint64_t length;
    int64_t offset;
} mmap_range_global_t;

typedef struct mmap_range_local_s {
    pagemap_t* pagemap;
    mmap_range_global_t* global;
    uint64_t base;
    uint64_t length;
    int64_t offset;
    uint64_t prot;
    uint64_t flags;
} mmap_range_local_t;

bool mmap_map_range(pagemap_t* pagemap, uint64_t virt, uint64_t phys, uint64_t size, uint64_t prot, uint64_t flags);
bool mmap_map_page_in_range(mmap_range_global_t* global_range, uint64_t virt, uint64_t phys, uint64_t prot);
pagemap_t* mmap_fork_pagemap(pagemap_t* old_pagemap);

bool munmap(pagemap_t* pagemap, uint64_t base, uint64_t length);
