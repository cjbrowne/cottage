#include "smp.h"
#include <stddef.h>
#include <klog/klog.h>
#include <cpu/atomic.h>
#include <malloc.h>
#include <gdt/gdt.h>
#include <interrupt/idt/idt.h>

uint32_t bsp_lapic_id = 0;
local_cpu_t** local_cpus;

void smp_init(struct limine_smp_response* smp_response)
{
    if(smp_response == NULL)
    {
        klog("smp", "SMP Disabled: No response from limine");
        return;
    }
    klog("smp", "BSP LAPIC ID: %x", smp_response->bsp_lapic_id);
    klog("smp", "Total CPU count: %d", smp_response->cpu_count);

    struct limine_smp_info** smp_info_array = smp_response->cpus;
    bsp_lapic_id = smp_response->bsp_lapic_id;

    local_cpus = (local_cpu_t**) malloc(sizeof(local_cpu_t*) * smp_response->cpu_count);

    for (uint64_t i = 0; i < smp_response->cpu_count; i++)
    {
        local_cpu_t* local_cpu = malloc(sizeof(local_cpu_t));
        local_cpus[i] = local_cpu;

        struct limine_smp_info* smp_info = smp_info_array[i];

        smp_info->extra_argument = (uint64_t)local_cpu;

        local_cpu->cpu_number = i;

        // don't stall the current CPU, we still need it to finish booting!
        if (smp_info->lapic_id == smp_response->bsp_lapic_id)
        {
            cpu_init(smp_info);
            continue;
        }

        smp_info->goto_address = cpu_init;

        // stall while offline
        while (atomic_load(&local_cpu->online) == 0);
    }

    klog("smp", "All CPUs brought online");
}

void cpu_init(struct limine_smp_info* smp_info)
{
    local_cpu_t* local_cpu = (local_cpu_t*)smp_info->extra_argument;
    // uint64_t cpu_number = local_cpu->cpu_number;
    local_cpu->lapic_id = smp_info->lapic_id;

    gdt_reload();
    idt_reload();
}
