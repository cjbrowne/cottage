#include "scheduler.h"
#include <stdint.h>
#include <interrupt/idt/idt.h>
#include <klog/klog.h>

uint8_t scheduler_vector;

void scheduler_isr(uint32_t num, cpu_status_t* status);

void scheduler_init()
{
    scheduler_vector = idt_allocate_vector();
    klog("sched", "Allocated scheduler vector 0x%x", scheduler_vector);

    interrupt_table[scheduler_vector] = (void*)scheduler_isr;
}

void scheduler_isr(__attribute__((unused)) uint32_t num, __attribute__((unused)) cpu_status_t* status)
{
    klog("sched", "Interrupt %x fired", num);
}

