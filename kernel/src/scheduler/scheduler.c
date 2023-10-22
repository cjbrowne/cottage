#include "scheduler.h"
#include <stdint.h>
#include <interrupt/idt/idt.h>
#include <klog/klog.h>
#include <mem/malloc.h>
#include <mem/vmm.h>
#include <cpu/smp.h>
#include <interrupt/apic/apic.h>

#define MAX_THREADS 512

_Atomic uint8_t scheduler_vector;
process_t* kernel_process;
thread_t* scheduler_running_queue[MAX_THREADS];
uint64_t working_cpus = 0;

void scheduler_isr(uint32_t num, cpu_status_t* status);

void scheduler_init()
{
    scheduler_vector = idt_allocate_vector();
    klog("sched", "Allocated scheduler vector 0x%x", scheduler_vector);

    interrupt_table[scheduler_vector] = (void*)scheduler_isr;
    set_ist(scheduler_vector, 1);

    kernel_process = malloc(sizeof(process_t));
    kernel_process->pagemap = &g_kernel_pagemap;
}

void scheduler_isr(__attribute__((unused)) uint32_t num, __attribute__((unused)) cpu_status_t* status)
{
    klog("sched", "Interrupt %x fired", num);
    panic("Scheduler ISR not yet implemented");
}

void scheduler_await()
{
    // disable interrupts - we can't be interrupted while trying to set up an
    // interrupt otherwise really bad things will happen
    asm volatile ("cli");
    local_cpu_t* local_cpu = cpu_get_current();    
    // wait for 20ms
    lapic_timer_oneshot(local_cpu, scheduler_vector, 20000);
    // enable interrupts and run a HLT loop until the interrupt fires
    asm volatile (
        "sti\n"
        "1:\n"
        "hlt\n"
        "jmp 1\n"
        : : :  "memory"
    );
}
