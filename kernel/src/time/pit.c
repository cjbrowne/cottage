#include "pit.h"
#include <io/io.h>

uint16_t pit_get_current_count()
{
    outb(0x43, 0);
    uint8_t lo = inb(0x40);
    uint8_t hi = inb(0x40);
    return ((uint16_t)hi << 8 | lo);
}

void pit_set_reload_value(uint16_t new_count)
{
    outb(0x43, 0x34);
    outb(0x40, (uint8_t)new_count);
    outb(0x40, (uint8_t)new_count >> 8);
}
