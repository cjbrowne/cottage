#include "isr.h"
#include <stdint.h>
#include <interrupt/idt/idt.h>
#include <gdt/gdt.h>
#include <klog/klog.h>
#include <panic.h>

// names of each type of exception the CPU can produce
static const char *exception_names[] = {
    "Divide by Zero Error",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"};

void isr_initialize_gates();

extern void interrupt_service_routines(void);

void handle_pagefault(__attribute__((unused)) uint32_t num, __attribute__((unused)) cpu_status_t* cpu_state)
{
    klog("isr", "Handling pagefault");
}

void handle_abort(__attribute__((unused)) uint32_t num, __attribute__((unused)) cpu_status_t* cpu_state)
{
    panic("Kernel caught ABORT interrupt");
}

void handle_exception(uint32_t num, __attribute__((unused)) cpu_status_t* cpu_state)
{
    if(cpu_state->cs == USER_CODE_SEGMENT)
    {
        // todo: send SIGSEGV (refer to VINIX example)
        panic("Can't send SIGSEGV yet");
    }
    else
    {
        panic("Caught exception %s in kernel (aka 0x%x)", exception_names[num], num);
    }
}

void handle_interrupt(uint32_t num, __attribute__((unused)) cpu_status_t* cpu_state)
{
    panic("Caught interrupt %d, not handled yet", num);
}

void isr_init() {
    uint64_t* isrs = (uint64_t*)((void*)interrupt_service_routines);
    for (uint16_t i = 0; i < 32; i++)
    {
        switch (i)
        {
            case 14:
            {
                set_idt_entry(i, 0x8e, KERNEL_CODE_SEGMENT, 3, (void (*)())isrs[i]);
                interrupt_table[i] = (void*)handle_pagefault;
                break;
            }
            default:
            {
                set_idt_entry(i, 0x8e, KERNEL_CODE_SEGMENT, 0, (void (*)())isrs[i]);
                interrupt_table[i] = (void*)handle_exception;
                break;
            }
        }
    }

    for (uint16_t i = 32; i < 256; i++)
    {
        set_idt_entry(i, 0x8e, KERNEL_CODE_SEGMENT, 0, (void (*)())isrs[i]);
        interrupt_table[i] = (void*)handle_interrupt;
    }

    uint16_t abort_vector = idt_allocate_vector();
    set_idt_entry(abort_vector, 0x8e, KERNEL_CODE_SEGMENT, 4, (void (*)())isrs[abort_vector]);
    interrupt_table[abort_vector] = (void*)handle_abort;
}
