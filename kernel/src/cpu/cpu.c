#include <cpu/cpu.h>

uint64_t fpu_storage_size;
// function pointers masquerading as naked pointers
void* fpu_save;
void* fpu_restore;

void xsave(void* region)
{
    asm volatile (
        "xsave %0"
        : : "m" (region),
        "a" (0xffffffff),
        "d" (0xffffffff)
        : "memory"
    );
}

void xrstor(void* region)
{
    asm volatile (
        "xrstor %0"
        : : "m" (region),
        "a" (0xffffffff),
        "d" (0xffffffff)
        : "memory"
    );
}

void fxsave(void* region)
{
    asm volatile (
        "fxsave %0"
        : : "m" (region)
        : "memory"
    );
}

void fxrstor(void* region)
{
    asm volatile (
        "fxrstor %0"
        : : "m" (region)
        : "memory"
    );
}
