#pragma once

#include <stdint.h>

static inline uint8_t inb(uint16_t port)
{
    // taken from wiki.osdev.org
    uint8_t ret;
    asm volatile("inb %1, %0"
                 : "=a"(ret)
                 : "Nd"(port)
                 : "memory");
    return ret;
}

static inline void outb(uint16_t port, uint8_t data)
{
    // taken from wiki.osdev.org
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port) : "memory");
}

// also taken from wiki.osdev.org
static inline void io_wait(void)
{
    outb(0x80, 0);
}
