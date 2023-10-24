#pragma once

#include <stdint.h>

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("in %1, %0"
                 : "=a"(ret)
                 : "Nd"(port)
                 : "memory");
    return ret;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile("in %1, %0"
                 : "=a"(ret)
                 : "Nd"(port)
                 : "memory");
    return ret;
}

static inline uint32_t ind(uint16_t port)
{
    uint32_t ret;
    asm volatile("in %1, %0"
                 : "=a"(ret)
                 : "Nd"(port)
                 : "memory");
    return ret;
}

static inline void outb(uint16_t port, uint8_t data)
{
    asm volatile("out %0, %1" : : "a"(data), "Nd"(port) : "memory");
}

static inline void outw(uint16_t port, uint16_t data)
{
    asm volatile("out %0, %1" : : "a"(data), "Nd"(port) : "memory");
}

static inline void outd(uint16_t port, uint32_t data)
{
    asm volatile("out %0, %1" : : "a"(data), "Nd"(port) : "memory");
}

// also taken from wiki.osdev.org
static inline void io_wait(void)
{
    outb(0x80, 0);
}
