#include "kmalloc.h"
#include <stdbool.h>
#include <panic.h>
#include <math/si.h>

void *kmalloc(__attribute__((unused)) size_t len)
{
    panic("Kernel OOM");
    
    // technically not reachable, but prevents gcc emitting a bogus error
    return NULL; 
}

void kfree(__attribute__((unused)) void *ptr)
{
}
