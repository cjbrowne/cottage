#include "timer.h"
#include <acpi/acpi.h>
#include <lai/include/acpispec/tables.h>
#include <mem/pagemap.h>
#include <klog/klog.h>
#include <mem/pmm.h>
#include <debug/debug.h>

extern pagemap_t g_kernel_pagemap;

static hpet_t* hpet = 0;

void init_timer()
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
