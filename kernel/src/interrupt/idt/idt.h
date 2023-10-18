#pragma once

#include <stdint.h>

#define IDT_PRESENT_FLAG 0x80
#define IDT_INTERRUPT_TYPE_FLAG 0x0E
#define IDT_SEGMENT_SELECTOR 0x08

// Define Interrupts labels
#define DIVIDE_ERROR 0
#define DEBUG_EXC 1
#define NMI_INTERRUPT 2
#define BREAKPOINT 3
#define OVERFLOW 4
#define BOUND_RANGE_EXCEED 5
#define INVALID_OPCODE 6
#define DEV_NOT_AVL 7
#define DOUBLE_FAULT 8
#define COPROC_SEG_OVERRUN 9
#define INVALID_TSS 10
#define SEGMENT_NOT_PRESENT 11
#define STACK_SEGMENT_FAULT 12
#define GENERAL_PROTECTION 13
#define PAGE_FAULT 14
#define INT_RSV 15
#define FLOATING_POINT_ERR 16
#define ALIGNMENT_CHECK 17
#define MACHINE_CHECK 18
#define SIMD_FP_EXC 19

#define APIC_TIMER_INTERRUPT 32
#define APIC_SPURIOUS_INTERRUPT 255

#define KEYBOARD_INTERRUPT 33
#define PIT_INTERRUPT 34

typedef struct
{
    uint64_t ds;
    uint64_t es;
    uint64_t rax;    
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    uint64_t error_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((__packed__)) cpu_status_t;

typedef struct
{
    uint16_t offset_low; // offset bits 0..15
    uint16_t selector;   // a code segment selector in GDT or LDT
    uint8_t
        ist;                 // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t type_attributes; // gate type, dpl, and p fields
    uint16_t offset_mid;     // offset bits 16..31
    uint32_t offset_high;    // offset bits 32..63
    uint32_t zero;           // reserved
} __attribute__((packed)) interrupt_descriptor_t;

typedef struct
{
    uint16_t limit;
    interrupt_descriptor_t *ptr;
} __attribute__((packed)) idtd_t;

// the kernel's code segment selector
void idt_init(uint16_t kernel_segment_selector);

// default interrupt service routine
void default_isr();

// forward declare all the asm-implemented ISRs
extern void interrupt_service_routine_0();
extern void interrupt_service_routine_1();
extern void interrupt_service_routine_2();
extern void interrupt_service_routine_3();
extern void interrupt_service_routine_4();
extern void interrupt_service_routine_5();
extern void interrupt_service_routine_6();
extern void interrupt_service_routine_7();
extern void interrupt_service_routine_error_code_8();
extern void interrupt_service_routine_9();
extern void interrupt_service_routine_error_code_10();
extern void interrupt_service_routine_error_code_11();
extern void interrupt_service_routine_error_code_12();
extern void interrupt_service_routine_error_code_13();
extern void interrupt_service_routine_error_code_14();
extern void interrupt_service_routine_15();
extern void interrupt_service_routine_16();
extern void interrupt_service_routine_error_code_17();
extern void interrupt_service_routine_18();
extern void interrupt_service_routine_32();
extern void interrupt_service_routine_33();
extern void interrupt_service_routine_34();
extern void interrupt_service_routine_255();
