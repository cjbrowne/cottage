#include "timer.h"
#include <acpi/acpi.h>
#include <lai/include/acpispec/tables.h>
#include <mem/pagemap.h>
#include <klog/klog.h>
#include <mem/pmm.h>
#include <debug/debug.h>
#include <lock/lock.h>
#include <time/pit.h>

extern pagemap_t g_kernel_pagemap;

static hpet_t* hpet = 0;
timespec_t monotonic_clock;
timespec_t realtime_clock;
lock_t timers_lock;
timer_t* armed_timers;
size_t armed_timers_count;

void hpet_init()
{
    // find the HPET table
    hpet_table_t* hpet_table = acpi_find_table("HPET", 0);
    
    klog("timer", "Reading HPET table at %x", hpet_table);

    hpet = (hpet_t*) hpet_table->address;
    
    klog("timer", "Found HPET memory region at %x", hpet);

    // identity map the HPET table into virtual memory
    map_page(&g_kernel_pagemap, (uint64_t)hpet, (uint64_t)hpet, PTE_FLAG_PRESENT | PTE_FLAG_WRITABLE);

    // start the timer running, and enable timer interrupts
    hpet->general_config = 1;
}

void timer_init(int64_t epoch)
{
    monotonic_clock = (timespec_t) {
        .tv_sec = epoch,
        .tv_nsec = 0
    };
    realtime_clock = (timespec_t) {
        .tv_sec = epoch,
        .tv_nsec = 0,
    };

    pit_init();
}

void sleep(uint32_t millis)
{
    millis /= 8;
    uint64_t deadline = hpet->counter_value + 
        // convert millis to ticks (might be better to use get_ticks_per_second?)
        (millis * 10000000000000)  
        / 
        ((hpet->capabilities >> 32) & 0xffffffff)
        ;

    // wait for deadline to pass
    while(hpet->counter_value < deadline)
        asm volatile ("pause" : : : "memory");
}

uint64_t get_ticks_per_second()
{
    return 1250000000000000 / ((hpet->capabilities >> 32) & 0xffffffff);
}

uint64_t get_ticks()
{
    return hpet->counter_value;
}

void add_interval(timespec_t* dest, timespec_t src)
{
    // handle rolling over nanoseconds
    if(dest->tv_nsec + src.tv_nsec > 999999999)
    {
       int64_t diff = (dest->tv_nsec + src.tv_nsec) - 1000000000;
       dest->tv_nsec = diff;
       dest->tv_sec++;
    } else {
        dest->tv_nsec += src.tv_nsec;
    }
    dest->tv_sec += src.tv_sec;
}

bool sub_interval(timespec_t* dest, timespec_t src)
{
    // if we're subtracting more than 1 second worth of nanoseconds,
    // handle the rollover
    if (src.tv_nsec > dest->tv_nsec)
    {
        int64_t diff = src.tv_nsec - dest->tv_nsec;
        dest->tv_nsec = 999999999 - diff;
        // if we have 0 seconds left and have rolled over,
        // we are at 0 and the timer is finished
        if (dest->tv_sec == 0)
        {
            dest->tv_sec = 0;
            dest->tv_nsec = 0;
            return true;
        }
        dest->tv_sec--;
    } else {
        dest->tv_nsec -= src.tv_nsec;
    }
    // if the number of seconds left on the timer is less than
    // the number we are trying to subtract, we are at zero and
    // the timer is finished
    if (src.tv_sec > dest->tv_sec)
    {
        dest->tv_sec = 0;
        dest->tv_nsec = 0;
        return true;
    }
    dest->tv_sec -= src.tv_sec;
    if (dest->tv_sec == 0 && dest->tv_nsec == 0)
    {
        return true;
    }
    return false;
}

void timer_handler()
{
    timespec_t interval = {
        .tv_sec = 0,
        .tv_nsec = 1000000000 / TIMER_FREQUENCY
    };

    add_interval(&monotonic_clock, interval);
    add_interval(&realtime_clock, interval);

    if (lock_test_and_acquire(&timers_lock))
    {
        for(size_t i = 0; i < armed_timers_count; i++)
        {
            timer_t t = armed_timers[i];
            if (t.fired) continue;
            if (sub_interval(&t.when, interval))
            {
                event_trigger(&t.event, false);
                t.fired = true;
            }
        }

        lock_release(&timers_lock);
    }
}
