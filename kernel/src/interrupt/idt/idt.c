#include "idt.h"
#include <interrupt/isr/isr.h>
#include <klog/klog.h>
#include <panic.h>
#include <stddef.h>
#include <stdio.h>
#include <mem/pagemap.h>

// this is *always* 256.  Do not change!
#define IDT_ENTRY_COUNT 256

// the descriptor table
interrupt_descriptor_t idt[IDT_ENTRY_COUNT];

// the interrupt vector table (used by ISRs to look up which vector to call)
void* interrupt_table[IDT_ENTRY_COUNT];

extern void interrupt_service_routines(void);

// names of each type of exception the CPU can produce
__attribute__((unused)) static const char *exception_names[] = {
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

// the idt descriptor is a pointer to the table,
// including a length parameter
static idtd_t idtd = {
    .limit = sizeof(idt) - 1,
    .ptr = idt,
};

void set_idt_entry(uint16_t idx, uint8_t flags, uint16_t selector, uint8_t ist,
                   void (*handler)());

cpu_status_t *interrupts_handler(uint32_t num, cpu_status_t *status)
{
    klog("idt", "Handling interrupt %d", num);

    switch (num)
    {
    case PAGE_FAULT:
    {
        klog("int", "Page fault %x", status->error_code);
        klog("int", "Virtual Address %x", read_cr2());
        panic("Page fault");
        break;
    }
    case APIC_TIMER_INTERRUPT:
    {
        klog("int", "Timer"); // expecting this to be very very spammy
        break;
    }
    default:
    {
        char buf[128];
        snprintf(buf, 128, "interrupt %d not implemented", num);
        panic(buf);
        break;
    }
    }

    // todo: implement me
    return status;
}

void idt_init(uint16_t kernel_segment_selector)
{
    int i = 0;
    for (i = 0; i < IDT_ENTRY_COUNT; i++)
    {
        idt[i].type_attributes = 0;
        idt[i].ist = 0;
        idt[i].offset_high = 0;
        idt[i].offset_mid = 0;
        idt[i].zero = 0;
        idt[i].offset_low = 0;
        idt[i].selector = 0;
    }

    uint64_t* isrs = (uint64_t*)(interrupt_service_routines);

    for (uint16_t i = 0; i < 256; i++)
    {
        set_idt_entry(i, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, kernel_segment_selector, 0, (void (*)())(isrs[i]));
        interrupt_table[i] = (void*)interrupts_handler;
    }

    klog("idt", "Loading IDT descriptor at %x, IDT table is at %x", &idtd,
         idtd.ptr);

    // load the IDT
    asm volatile("lidt %0" : : "g"(idtd));
}

void set_idt_entry(uint16_t idx, uint8_t flags, uint16_t selector, uint8_t ist,
                   void (*handler)())
{
    idt[idx].type_attributes = flags;
    idt[idx].ist = ist;
    idt[idx].selector = selector;
    // convert a 64 bit address into three offset values for the fucky way the IDT
    // works
    idt[idx].offset_low = (uint16_t)((uint64_t)handler & 0xFFFF);
    idt[idx].offset_mid = (uint16_t)((uint64_t)handler >> 16);
    idt[idx].offset_high = (uint32_t)((uint64_t)handler >> 32);

    // make sure this is always set to 0
    idt[idx].zero = 0;
}
