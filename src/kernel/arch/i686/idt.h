#pragma once

#include <stdint.h>

typedef enum {
    IDT_FLAG_GATE_TASK                  = 0x5,
    IDT_FLAG_GATE_16BIT_INT             = 0x6,      // 16-bit mode interrupt gate (disables interrupts, returns to next instruction)
    IDT_FLAG_GATE_16BIT_FLAG            = 0x7,      // 16-bit mode trap gate (leaves interrupts enabled, returns to current instruction)
    IDT_FLAG_GATE_32BIT_INT             = 0xE,      // 32-bit mode interrupt gate (disables interrupts, returns to next instruction)
    IDT_FLAG_GATE_32BIT_FLAG            = 0xF,      // 32-bit mode trap gate (leaves interrupts enabled, returns to current instruction)

    // which ring can call this interrupt
    IDT_FLAG_RING0                      = (0 << 5),
    IDT_FLAG_RING1                      = (1 << 5),
    IDT_FLAG_RING2                      = (2 << 5),
    IDT_FLAG_RING3                      = (3 << 5),

    IDT_FLAG_PRESENT                    = 0x80,      // enable/disable bit

} IDT_FLAGS;

void i686_IDT_init();
void i686_IDT_EnableGate(int interrupt);
void i686_IDT_DisableGate(int interrupt);
void i686_IDT_SetGate(int interrupt, void* base, uint16_t segment_descriptor, uint8_t flags);
