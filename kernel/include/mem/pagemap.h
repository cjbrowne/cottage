#pragma once

#include <lock/lock.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define PTE_FLAG_PRESENT (uint64_t)(1 << 0)
#define PTE_FLAG_WRITABLE (uint64_t)(1 << 1)
#define PTE_FLAG_USER (uint64_t)(1 << 2)

typedef struct {
    void* top_level;
    void** mmap_ranges;
    size_t mmap_range_count;
    lock_t lock;
} pagemap_t;

// couple of helper functions to map multi-page regions
uint64_t find_contiguous_pages(pagemap_t* pagemap, size_t count);
bool map_contiguous_pages(pagemap_t* pagemap, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags, size_t count);

// the bulk of the actual page mapping functions
pagemap_t new_pagemap();
bool map_page(pagemap_t* pagemap, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);
void switch_pagemap(pagemap_t* pagemap);
uint64_t* virt2pte(pagemap_t* pagemap, uint64_t virt_addr, bool allocate);
bool virt2phys(pagemap_t* pagemap, uint64_t virt_addr, uint64_t* phys);
bool unmap_page(pagemap_t* pagemap, uint64_t virt);
bool flag_page(pagemap_t* pagemap, uint64_t virt, uint64_t flags);
bool delete_pagemap(pagemap_t* pagemap);

static inline uint64_t read_cr0()
{
    uint64_t ret = 0;
    asm volatile (
        "mov %%cr0, %0"
        : "=r" (ret)
        : : "memory"
    );
    return ret;
}

static inline uint64_t read_cr1()
{
    uint64_t ret = 0;
    asm volatile (
        "mov %%cr1, %0"
        : "=r" (ret)
        : : "memory"
    );
    return ret;
}

// CR2 is an important pagemap-related register
static inline uint64_t read_cr2()
{
    uint64_t ret = 0;
    asm volatile (
        "mov %%cr2, %0"
        : "=r" (ret)
        : : "memory"
    );
    return ret;
}

// CR3 is an important pagemap-related register
static inline uint64_t read_cr3()
{
    uint64_t ret = 0;
    asm volatile (
        "mov %%cr3, %0"
        : "=r" (ret)
        : : "memory"
    );
    return ret;
}

static inline uint64_t read_cr4()
{
    uint64_t ret = 0;
    asm volatile (
        "mov %%cr4, %0"
        : "=r" (ret)
        : : "memory"
    );
    return ret;
}

static inline void write_cr0(uint64_t val)
{
    asm volatile (
        "mov %0, %%cr0"
        : : "r" (val)
        : "memory"
    );
}

static inline void write_cr1(uint64_t val)
{
    asm volatile (
        "mov %0, %%cr1"
        : : "r" (val)
        : "memory"
    );
}

static inline void write_cr2(uint64_t val)
{
    asm volatile (
        "mov %0, %%cr2"
        : : "r" (val)
        : "memory"
    );
}

static inline void write_cr3(uint64_t val)
{
    asm volatile (
        "mov %0, %%cr3"
        : : "r" (val)
        : "memory"
    );
}

static inline void write_cr4(uint64_t val)
{
    asm volatile (
        "mov %0, %%cr4"
        : : "r" (val)
        : "memory"
    );
}

// INVLPG is an instruction used to invalidate a page at <virt>
static inline void invlpg(uint64_t virt)
{
    asm volatile (
        "invlpg (%0)"
        :
        : "r" (virt)
        : "memory"
    );
}
