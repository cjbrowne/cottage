#include "isr.h"
#include "idt.h"
#include "io.h"
#include <stdio.h>
#include <stddef.h>

// global interrupt service routine handlers
ISRHandler g_ISRHandlers[256];

// exception list
static const char *const g_Exceptions[] = {
        "Divide by zero error",
        "Debug",
        "Non-maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "Bound Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection Fault",
        "Page Fault",
        "",
        "x87 Floating-Point Exception",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
        "Control Protection Exception ",
        "",
        "",
        "",
        "",
        "",
        "",
        "Hypervisor Injection Exception",
        "VMM Communication Exception",
        "Security Exception",
        ""
};

void i686_ISR_InitializeGates();

void i686_ISR_init() {
    i686_ISR_InitializeGates();
    for (int i = 0; i < 256; i++)
        i686_IDT_EnableGate(i);

    i686_IDT_DisableGate(0x80);
}

void __attribute__((cdecl)) i686_ISR_Handler(registers *regs) {
    if (g_ISRHandlers[regs->interrupt_number] != NULL) {
        g_ISRHandlers[regs->interrupt_number](regs);
    } else if (regs->interrupt_number >= 32) {
        printf("ISR: Unhandled interrupt %d\n", regs->interrupt_number);
    } else {
        printf("ISR: Unhandled exception %d %s\n", regs->interrupt_number, g_Exceptions[regs->interrupt_number]);

        printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        printf("  interrupt=%x  errorcode=%x\n", regs->interrupt_number, regs->error);

        printf("KERNEL PANIC!\n");
        i686_Panic();
    }
}

void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler)
{
    g_ISRHandlers[interrupt] = handler;
    i686_IDT_EnableGate(interrupt);
}