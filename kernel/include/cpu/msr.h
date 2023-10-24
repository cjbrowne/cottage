#pragma once

#include <stdint.h>

static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t eax = 0, edx = 0;
    asm volatile (
        "rdmsr"
        :   "=a" (eax),
            "=d" (edx)
        : "c" (msr)
        : "memory"
    );
    return ((uint64_t)edx << 32) | eax;
}

static inline void wrmsr(uint32_t msr, uint64_t value)
{
    uint32_t eax = value;
    uint32_t edx = (value >> 32);
    asm volatile (
        "wrmsr"
        : : "a" (eax),
            "d" (edx),
            "c" (msr)
        : "memory"
    );
}
