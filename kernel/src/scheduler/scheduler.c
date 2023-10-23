#include "scheduler.h"
#include <stdint.h>
#include <interrupt/idt/idt.h>
#include <klog/klog.h>
#include <mem/malloc.h>
#include <mem/vmm.h>
#include <mem/pmm.h>
#include <cpu/smp.h>
#include <cpu/cpu.h>
#include <mem/align.h>
#include <gdt/gdt.h>
#include <interrupt/apic/apic.h>
#include <string.h>

// use 2MB stack, similar to Linux
#define STACK_SIZE 0x200000LL
#define MAX_THREADS 512

_Atomic bool scheduler_ready = false;
_Atomic uint8_t scheduler_vector;
process_t* kernel_process;
_Atomic(thread_t*) scheduler_running_queue[MAX_THREADS];
uint64_t working_cpus = 0;

void scheduler_isr(uint32_t num, cpu_status_t* status);

void scheduler_init()
{
    klog("sched", "initialising scheduler");
    scheduler_vector = idt_allocate_vector();
    klog("sched", "Allocated scheduler vector 0x%x", scheduler_vector);
    interrupt_table[scheduler_vector] = (void*)((uint64_t)scheduler_isr);
    set_ist(scheduler_vector, 1);
    kernel_process = malloc(sizeof(process_t));
    kernel_process->pagemap = &g_kernel_pagemap;
    // let everyone know we're up and running
    atomic_store(&scheduler_ready, true);
}

void scheduler_isr(__attribute__((unused)) uint32_t num, __attribute__((unused)) cpu_status_t* status)
{
    klog("sched", "Interrupt %x fired", num);
    panic("Scheduler ISR not yet implemented");
}

void scheduler_await()
{
    // disable interrupts - we can't be interrupted while using cpu_get_current
    // or else we might get an incorrect or inconsistent result
    asm volatile ("cli");
    CHECKPOINT
    local_cpu_t* local_cpu = cpu_get_current();
    CHECKPOINT
    // wait for 20ms
    lapic_timer_oneshot(local_cpu, scheduler_vector, 20000);
    // enable interrupts and run a HLT loop until the interrupt fires
    asm volatile ("sti");
    for(;;)
    {
        asm volatile("hlt":::"memory");
    }
}

bool enqueue_thread(thread_t* thread, bool by_signal)
{
    // shortcut for duplicate calls
    if (thread->is_in_queue) return true;

    atomic_store(&thread->enqueued_by_signal, by_signal);

    for (uint64_t i = 0; i < MAX_THREADS; i++)
    {
        thread_t* expected = NULL;
        if (atomic_compare_exchange_strong(&scheduler_running_queue[i], &expected, thread))
        {
            thread->is_in_queue = true;

            // wake up any idle CPUs
            for(uint64_t i = 0; i < cpu_count; i++)
            {
                if (atomic_load(&local_cpus[i]->is_idle))
                {
                    lapic_send_ipi(local_cpus[i]->lapic_id, scheduler_vector);
                    break;
                }
            }

            // we did manage to enqueue this thread
            return true;
        }
    }

    // we could not enqueue this thread
    return false;
}

thread_t* new_kernel_thread(void* ip, void* arg, bool autoenqueue)
{
    void* stacks[PROC_MAX_STACKS_PER_THREAD];
    size_t stack_count = 0;
    void* stack_phys = pmm_alloc(STACK_SIZE / PAGE_SIZE);
    stacks[stack_count++] = stack_phys;
    uint64_t stack = ((uint64_t)stack_phys) + STACK_SIZE + HIGHER_HALF;

    cpu_status_t cpu_state = {
        .cs = KERNEL_CODE_SEGMENT,
        .ds = KERNEL_DATA_SEGMENT,
        .es = KERNEL_DATA_SEGMENT,
        .ss = KERNEL_DATA_SEGMENT,
        .rflags = 0x202,
        .rip = (uint64_t)ip,
        .rdi = (uint64_t)arg,
        .rbp = (uint64_t)0,
        .rsp = stack
    };

    thread_t* t = malloc(sizeof(thread_t));
    t->process = kernel_process;
    t->cr3 = (uint64_t) kernel_process->pagemap->top_level;
    t->cpu_state = cpu_state;
    t->timeslice = 5000;
    t->cpuid = -1;
    memcpy(t->stacks, stacks, sizeof(stacks));
    t->fpu_storage = (void*) ((uint64_t)pmm_alloc(div_roundup(fpu_storage_size, PAGE_SIZE)) + HIGHER_HALF);
    t->self = t;
    t->gs_base = (uint64_t) t;
    if (autoenqueue)
    {
        if(!enqueue_thread(t, false))
        {
            panic("Kernel could not enqueue a thread, did we hit MAX_THREADS for kernel threads?");
        }
    }

    return t;
}
