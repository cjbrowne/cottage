#include "malloc.h"
#include <stdbool.h>
#include <mem/slaballoc.h>
#include <mem/pmm.h>
#include <mem/align.h>

void *malloc(size_t len)
{
    slab_t* slab = find_slab(8 + len);
    if(slab == NULL)
    {
        return big_malloc(len);
    }
    else
    {
        return slab_alloc(slab);
    }
}

void* big_malloc(size_t len)
{
    uint64_t page_count = div_roundup(len, PAGE_SIZE);

    void* ptr = pmm_alloc(page_count + 1);

    if (ptr == NULL)
    {
        return NULL;
    }

    malloc_metadata_t* metadata = (malloc_metadata_t*) ((uint64_t)ptr + HIGHER_HALF);
    metadata->pages = page_count;
    metadata->size = len;

    return (void*)(((uint64_t)ptr) + HIGHER_HALF + PAGE_SIZE);
}

void free(void *ptr)
{
    if(ptr == NULL) return;

    if (((uint64_t)ptr & (uint64_t)0xfff) == 0)
    {
        big_free(ptr);
        return;
    }

    slabheader_t* slab_hdr = (slabheader_t*) ((uint64_t)ptr & ~(uint64_t)0xFFF);
    slab_free(slab_hdr->slab, ptr);
}

void big_free(void* ptr)
{
    malloc_metadata_t* metadata = (malloc_metadata_t*) (((uint64_t)ptr) - PAGE_SIZE);
    pmm_free((void*)((uint64_t)metadata - HIGHER_HALF), metadata->pages + 1);
}
