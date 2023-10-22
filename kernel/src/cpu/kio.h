#pragma once

#include <stdint.h>

#define mmin(x) _Generic((*x), \
    uint64_t: mmin64, \
    uint32_t: mmin32, \
    uint16_t: mmin16, \
    uint8_t: mmin8, \
    default: mmin64 \
)((uint64_t)x)

#define mmout(x, y) _Generic((y), \
    uint64_t: mmout64, \
    uint32_t: mmout32, \
    uint16_t: mmout16, \
    uint8_t: mmout8, \
    default: mmout64 \
)(x, y)

static inline uint64_t mmin64(uint64_t addr)
{
    return *((volatile uint64_t*) addr);
}

static inline uint32_t mmin32(uint64_t addr)
{
    return *((volatile uint32_t*) addr);
}

static inline uint16_t mmin16(uint64_t addr)
{
    return *((volatile uint16_t*) addr);
}

static inline uint8_t mmin8(uint64_t addr)
{
    return *((volatile uint8_t*) addr);
}

static inline void mmout64(void* addr, uint64_t value)
{
    (*((volatile uint64_t*) addr)) = value;
}

static inline void mmout32(void* addr, uint32_t value)
{
    (*((volatile uint32_t*) addr)) = value;
}

static inline void mmout16(void* addr, uint16_t value)
{
    (*((volatile uint16_t*) addr)) = value;
}

static inline void mmout8(void* addr, uint8_t value)
{
    (*((volatile uint8_t*) addr)) = value;
}
