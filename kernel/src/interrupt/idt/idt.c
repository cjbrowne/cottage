#include "idt.h"
#include <interrupt/isr/isr.h>
#include <klog/klog.h>
#include <panic.h>
#include <stddef.h>
#include <stdio.h>

// this is *always* 256.  Do not change!
#define IDT_ENTRY_COUNT 256

// the table itself
interrupt_descriptor_t idt[IDT_ENTRY_COUNT];

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

cpu_status_t *interrupts_handler(cpu_status_t *status)
{
    klog("idt", "Handling interrupt %d", status->interrupt_number);

    switch (status->interrupt_number)
    {
    case PAGE_FAULT:
    {
        klog("int", "Page fault %x", status->error_code);
        panic("Page fault");
        break;
    }
    default:
    {
        char buf[128];
        snprintf(buf, 128, "interrupt %ld not implemented", status->interrupt_number);
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

    set_idt_entry(0x00, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_0);
    set_idt_entry(0x01, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_1);
    set_idt_entry(0x02, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_2);
    set_idt_entry(0x03, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_3);
    set_idt_entry(0x04, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_4);
    set_idt_entry(0x05, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_5);
    set_idt_entry(0x06, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_6);
    set_idt_entry(0x07, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_7);
    set_idt_entry(0x08, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0,
                  interrupt_service_routine_error_code_8);
    set_idt_entry(0x09, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_9);
    set_idt_entry(0x0A, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0,
                  interrupt_service_routine_error_code_10);
    set_idt_entry(0x0B, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0,
                  interrupt_service_routine_error_code_11);
    set_idt_entry(0x0C, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0,
                  interrupt_service_routine_error_code_12);
    set_idt_entry(0x0D, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0,
                  interrupt_service_routine_error_code_13);
    set_idt_entry(0x0E, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0,
                  interrupt_service_routine_error_code_14);
    set_idt_entry(0x0F, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_15);
    set_idt_entry(0x10, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_16);
    set_idt_entry(0x11, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0,
                  interrupt_service_routine_error_code_17);
    set_idt_entry(0x12, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_18);
    set_idt_entry(0x20, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_32);
    set_idt_entry(0x21, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_33);
    set_idt_entry(0x22, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_34);
    set_idt_entry(0xFF, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG,
                  kernel_segment_selector, 0, interrupt_service_routine_255);

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
