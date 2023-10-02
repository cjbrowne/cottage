#pragma once

#include <stdint.h>

/**
* Kernel Interrupt definitions:
 *
*/

typedef struct {
    // in the reverse order they are pushed
    uint32_t ds;                                            // data segment
    uint32_t edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;   // pusha
    uint32_t interrupt_number;                              // int number
    uint32_t error;                                         // error code (if present)
    uint32_t eip, cs, eflags, esp, ss;                      // pushed automatically by CPU
} __attribute__((packed)) registers;

typedef void (*ISRHandler)(registers* regs);

void i686_ISR_init();
void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler);
