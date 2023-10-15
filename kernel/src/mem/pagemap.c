#include "pagemap.h"
#include <stdint.h>
#include <stdbool.h>
#include <mem/vmm.h>
#include <mem/pmm.h>

bool map_page(pagemap_t* pagemap, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags)
{
    // this is all just magic ways of extracting the page map index from the virtual address
    // don't put too much thought into it, as long as all the magic numbers are correct,
    // the incantation will work.
    uint64_t pml4_entry = (virt_addr & (((uint64_t)0x1ff) << 39)) >> 39;
    uint64_t pml3_entry = (virt_addr & (((uint64_t)0x1ff) << 30)) >> 30;
    uint64_t pml2_entry = (virt_addr & (((uint64_t)0x1ff) << 21)) >> 21;
    uint64_t pml1_entry = (virt_addr & (((uint64_t)0x1ff) << 12)) >> 12;

    uint64_t* pml4 = pagemap->top_level;
    uint64_t* pml3 = get_next_level(pml4, pml4_entry, true);
    if(pml3 == 0) return false;
    uint64_t* pml2 = get_next_level(pml3, pml3_entry, true);
    if(pml2 == 0) return false;
    uint64_t* pml1 = get_next_level(pml2, pml2_entry, true);
    if(pml1 == 0) return false;

    uint64_t* entry = (uint64_t*)(((uint64_t)pml1) + HIGHER_HALF + pml1_entry * 8);
    entry[0] = phys_addr | flags;

    return true;
}

void switch_pagemap(pagemap_t* pagemap)
{
    void* top = pagemap->top_level;
    asm volatile (
        "mov %%cr3, %0"
        : /* no output */ 
        : "r" (top) 
        : "memory"
    );
}

uint64_t* virt2pte(pagemap_t* pagemap, uint64_t virt_addr, bool allocate)
{
    uint64_t pml4_entry = (virt_addr & (((uint64_t)0x1ff) << 39)) >> 39;
    uint64_t pml3_entry = (virt_addr & (((uint64_t)0x1ff) << 30)) >> 30;
    uint64_t pml2_entry = (virt_addr & (((uint64_t)0x1ff) << 21)) >> 21;
    uint64_t pml1_entry = (virt_addr & (((uint64_t)0x1ff) << 12)) >> 12;

    uint64_t* pml4 = pagemap->top_level;
    uint64_t* pml3 = get_next_level(pml4, pml4_entry, allocate);
    if(pml3 == 0) return NULL;
    uint64_t* pml2 = get_next_level(pml3, pml3_entry, allocate);
    if(pml2 == 0) return NULL;
    uint64_t* pml1 = get_next_level(pml2, pml2_entry, allocate);
    if(pml1 == 0) return NULL;

    return (uint64_t*) (((uint64_t)&pml1[pml1_entry]) + HIGHER_HALF);
}

bool unmap_page(pagemap_t* pagemap, uint64_t virt)
{
    uint64_t* pte_p =  virt2pte(pagemap, virt, false);
    if(pte_p == NULL)
    {
        return false;
    }

    *pte_p = 0;

    uint64_t current_cr3 = read_cr3();
    if(current_cr3 == (uint64_t)pagemap->top_level) {
        invlpg(virt);
    }
    return true;
}

bool flag_page(pagemap_t* pagemap, uint64_t virt, uint64_t flags)
{
    uint64_t* pte_p =  virt2pte(pagemap, virt, false);
    if(pte_p == NULL)
    {
        return false;
    }

    *pte_p &= ~((uint64_t)0xfff);
    *pte_p |= flags;

    uint64_t current_cr3 = read_cr3();
    if(current_cr3 == (uint64_t)pagemap->top_level) {
        invlpg(virt);
    }
    return true;
}
