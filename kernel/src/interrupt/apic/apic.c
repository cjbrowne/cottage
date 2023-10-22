#include "apic.h"
#include <klog/klog.h>
#include <cpu/msr.h>
#include <mem/pmm.h>
#include <cpu/kio.h>
#include <stdint.h>
#include <cpu/cpu.h>
#include <time/pit.h>
#include <cpu/smp.h>

#define LAPIC_REG_ICR0 0x300
#define LAPIC_REG_ICR1 0x310
#define LAPIC_REG_SPURIOUS 0x0f0
#define LAPIC_REG_EOI 0x0b0
#define LAPIC_REG_TIMER 0x320
#define LAPIC_REG_TIMER_INITCNT 0x380
#define LAPIC_REG_TIMER_CURCNT 0x390
#define LAPIC_REG_TIMER_DIV 0x3e0

static uint64_t lapic_base = 0;

uint32_t lapic_read(uint32_t reg)
{
   if (lapic_base == 0)
   {
        lapic_base = (rdmsr(0x1b) & 0xfffff000) + HIGHER_HALF;
   } 
   return mmin((void*)(lapic_base + reg));
}

void lapic_write (uint32_t reg, uint32_t val)
{
   if (lapic_base == 0)
   {
        lapic_base = (rdmsr(0x1b) & 0xfffff000) + HIGHER_HALF;
   } 
   return mmout((void*)(lapic_base + reg), val);
}

void lapic_enable(uint16_t spurious_vector)
{
    lapic_write(LAPIC_REG_SPURIOUS, lapic_read(LAPIC_REG_SPURIOUS) | (1 << 8) | spurious_vector);
}

void lapic_timer_stop()
{
    lapic_write(LAPIC_REG_TIMER_INITCNT, 0);
    lapic_write(LAPIC_REG_TIMER, (1 << 16));
}

void lapic_timer_calibrate(local_cpu_t local_cpu)
{
    lapic_timer_stop();
    uint64_t samples = 0xfffff;
    lapic_write(LAPIC_REG_TIMER, (1 << 16) | 0xff); // vector 0xff, masked
    lapic_write(LAPIC_REG_TIMER_DIV, 0);

    pit_set_reload_value(0xffff);

    uint64_t initial_pit_tick = pit_get_current_count();

    lapic_write(LAPIC_REG_TIMER_INITCNT, (uint32_t)samples);

    while(lapic_read(LAPIC_REG_TIMER_CURCNT) != 0);

    uint64_t final_pit_tick = (uint64_t) pit_get_current_count();

    uint64_t pit_ticks = initial_pit_tick - final_pit_tick;

    local_cpu.lapic_timer_freq = (samples / pit_ticks) * PIT_DIVIDEND;
    
    lapic_timer_stop();
}

void lapic_timer_oneshot(local_cpu_t* local_cpu, uint8_t vector, uint64_t micros)
{
    lapic_timer_stop();

    uint64_t ticks = micros * (local_cpu->lapic_timer_freq / 1000000);
    
    lapic_write(LAPIC_REG_TIMER, vector);
    lapic_write(LAPIC_REG_TIMER_DIV, 0);
    lapic_write(LAPIC_REG_TIMER_INITCNT, (uint32_t)ticks);
}
