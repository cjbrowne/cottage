#include "pit.h"
#include <io/io.h>
#include <interrupt/apic/apic.h>
#include <interrupt/idt/idt.h>
#include <interrupt/isr/isr.h>
#include <cpu/smp.h>
#include <time/timer.h>
#include <klog/klog.h>

void pit_set_frequency(uint64_t freq)
{
    uint64_t new_divisor = PIT_DIVIDEND / freq;
    if (PIT_DIVIDEND % freq > freq / 2)
    {
        new_divisor++;
    }

    pit_set_reload_value((uint16_t) new_divisor);
}

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

void pit_handler(__attribute__((unused)) uint32_t num, __attribute__((unused)) cpu_status_t* status)
{
    timer_handler();
    lapic_eoi();
}

void pit_init()
{
    pit_set_frequency(TIMER_FREQUENCY);

    uint8_t vector = idt_allocate_vector();
    klog("timer", "PIT Vector is 0x%x", vector);
    interrupt_table[vector] = (void*)pit_handler;
    io_apic_set_irq_redirect(local_cpus[0]->lapic_id, vector, 0, true);
}
