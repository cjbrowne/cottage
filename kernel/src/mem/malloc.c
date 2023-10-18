#include "malloc.h"
#include <stdbool.h>
#include <mem/slaballoc.h>
#include <mem/pmm.h>
#include <mem/align.h>
#include <string.h>

void* big_realloc(void* ptr, size_t len);

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

void* realloc(void* ptr, size_t len)
{
    if(ptr == NULL) return malloc(len);
    if((((uint64_t) ptr) & ((uint64_t) 0xfff)) == 0)
    {
        return big_realloc(ptr, len);
    }
    slabheader_t* slab_hdr = (slabheader_t*)(((uint64_t)ptr) & ~((uint64_t)0xfff));
    if (len > slab_hdr->slab->ent_size)
    {
        void* new_ptr = malloc(len);
        memcpy(new_ptr, ptr, slab_hdr->slab->ent_size);
        slab_free(slab_hdr->slab, ptr);
        return new_ptr;
    }

    return ptr;
}

void* big_realloc(void* ptr, size_t len)
{
    malloc_metadata_t* metadata = (malloc_metadata_t*)((uint64_t)ptr - PAGE_SIZE);
    if(div_roundup(metadata->size, PAGE_SIZE) == div_roundup(len, PAGE_SIZE))
    {
        // the easiest resize, we just tell it to use the rest of the page it
        // has already been allocated
        metadata->size = len;
        return ptr;
    }
    
    void* new_ptr = malloc(len);
    if (new_ptr == NULL) return NULL;


    // copy data from the old region to the new region
    if(metadata->size > len)
    {
        // in this mode, we will truncate the data (unavoidable, you're doing a big->small resize)
        memcpy(new_ptr, ptr, len);
    }
    else
    {
        // in this mode, we will preserve all source data
        memcpy(new_ptr, ptr, metadata->size);
    }

    free(ptr);

    return new_ptr;
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
