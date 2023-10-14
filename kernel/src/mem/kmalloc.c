#include "kmalloc.h"
#include <stdbool.h>
#include <panic.h>

// kernel requires at least 1 MiB to run
// feel free to tweak this, but be careful not to set it
// too high as you will make the kernel less portable
#define MIN_KMEM_SIZE MiB(1)

extern bool have_malloc;

size_t kmalloc_init(void *base, size_t size)
{
    if (size < MIN_KMEM_SIZE)
    {
        panic("not enough memory for kernel");
    }

    have_malloc = true;
}

void *kmalloc(size_t len)
{

    panic("Kernel OOM");
}

void kfree(void *ptr)
{
}
