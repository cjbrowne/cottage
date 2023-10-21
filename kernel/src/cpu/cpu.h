#pragma once

#include <stdbool.h>
#include <cpu/msr.h>

#define CPUID_XSAVE (1 << 26)
#define CPUID_AVX (1 << 28)
#define CPUID_AVX512 (1 << 16)

static inline void set_kernel_gs_base(uint64_t ptr)
{
    wrmsr(0xc0000102, ptr);
}

static inline void set_gs_base(uint64_t ptr)
{
    wrmsr(0xc0000101, ptr);
}

static inline void wrxcr(uint32_t reg, uint64_t val)
{
    uint32_t a = (uint32_t)val;
    uint32_t d = (uint32_t)(val >> 32);
    asm volatile (
        "xsetbv"
        : : "a" (a),
        "d" (d),
        "c" (reg)
        : "memory"
    );
}

void xsave(void* region);
void xrstor(void* region);
void fxsave(void* region);
void fxrstor(void* region);

static inline bool cpu_id(uint32_t leaf, uint32_t subleaf, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d)
{
    uint32_t cpuid_max = 0;
    asm volatile (
        "cpuid"
        : "=a" (cpuid_max)
        : "a" (leaf & 0x80000000)
        : "%rbx", "%rcx", "%rdx"
    );
    if (leaf > cpuid_max) {
        return false;
    }
    asm volatile(
        "cpuid"
        :   "=a" (*a),
            "=b" (*b),
            "=c" (*c),
            "=d" (*d)
            :   "a" (leaf),
                "c" (subleaf)
    );
    return true;
}

extern uint64_t fpu_storage_size;
// function pointers
extern void* fpu_save;
extern void* fpu_restore;
