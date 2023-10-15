#pragma once

#include <stdint.h>

// the number of slabs in the system
#define SLAB_COUNT 10

typedef struct {
    uint64_t first_free;
    uint64_t ent_size;
} slab_t;

typedef struct {
    slab_t* slab;
} slabheader_t;

void slaballoc_init();
void init_slab(slab_t* slab, uint64_t ent_size);
slab_t* find_slab(uint64_t size);
void* slab_alloc(slab_t* slab);
void slab_free(slab_t* slab, void* ptr);
