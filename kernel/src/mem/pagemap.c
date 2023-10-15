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
