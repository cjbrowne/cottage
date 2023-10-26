#include <cpu/cpu.h>


uint64_t fpu_storage_size;
// function pointers masquerading as naked pointers
fpuSaveFn fpu_save;
fpuRestoreFn fpu_restore;

void xsave(void* region)
{
    asm volatile (
        "xsave (%0)"
        : : "r" (region),
        "a" (0xffffffff),
        "d" (0xffffffff)
        : "memory"
    );
}

void xrstor(void* region)
{
    asm volatile (
        "xrstor (%0)"
        : : "r" (region),
        "a" (0xffffffff),
        "d" (0xffffffff)
        : "memory"
    );
}

void fxsave(void* region)
{
    asm volatile (
        "fxsave (%0)"
        : : "r" (region)
        : "memory"
    );
}

void fxrstor(void* region)
{
    asm volatile (
        "fxrstor (%0)"
        : : "r" (region)
        : "memory"
    );
}
