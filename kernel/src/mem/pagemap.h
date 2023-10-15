#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PTE_FLAG_PRESENT (uint64_t)(1 << 0)
#define PTE_FLAG_WRITABLE (uint64_t)(1 << 1)
#define PTE_FLAG_USER (uint64_t)(1 << 2)

typedef struct {
    void* top_level;
    void** mmap_ranges;
} pagemap_t;

bool map_page(pagemap_t* pagemap, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);
void switch_pagemap(pagemap_t* pagemap);
uint64_t* virt2pte(pagemap_t* pagemap, uint64_t virt_addr, bool allocate);
bool unmap_page(pagemap_t* pagemap, uint64_t virt);
bool flag_page(pagemap_t* pagemap, uint64_t virt, uint64_t flags);

static inline uint64_t read_cr2()
{
    uint64_t ret = 0;
    asm volatile (
        "mov %0, %%cr2"
        : "=r" (ret)
        : : "memory"
    );
    return ret;
}

static inline uint64_t read_cr3()
{
    uint64_t ret = 0;
    asm volatile (
        "mov %0, %%cr3"
        : "=r" (ret)
        : : "memory"
    );
    return ret;
}

static inline void invlpg(uint64_t virt)
{
    asm volatile (
        "invlpg (%0)"
        :
        : "r" (virt)
        : "memory"
    );
}
