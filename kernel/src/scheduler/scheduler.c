#include <scheduler/scheduler.h>
#include <stdint.h>
#include <interrupt/idt.h>
#include <klog/klog.h>
#include <mem/malloc.h>
#include <mem/vmm.h>
#include <mem/pmm.h>
#include <cpu/smp.h>
#include <cpu/cpu.h>
#include <mem/align.h>
#include <gdt/gdt.h>
#include <interrupt/apic.h>
#include <string.h>
#include <debug/debug.h>

// use 2MB stack, similar to Linux
#define STACK_SIZE (uint64_t)(0x200000)
#define MAX_THREADS 512

// variables
_Atomic bool scheduler_ready = false;
_Atomic uint8_t scheduler_vector;
process_t *kernel_process;
_Atomic(thread_t *) scheduler_running_queue[MAX_THREADS];
_Atomic uint64_t working_cpus = 0;

// functions
int64_t get_next_thread(int64_t orig_i);
void scheduler_isr(uint32_t num, cpu_status_t *status);

int64_t get_next_thread(int64_t orig_i)
{
    uint64_t cpu_number = cpu_get_current()->cpu_number;
    int64_t index = orig_i + 1;

    while (1)
    {
        // wrap-around
        if (index >= MAX_THREADS)
            index = 0;

        thread_t *t = scheduler_running_queue[index];
        if (t != 0)
        {
            if (atomic_load(&t->cpuid) == cpu_number || lock_test_and_acquire(&t->lock))
            {
                return index;
            }
        }

        // only allow looping through the full queue once
        if (index == orig_i)
            break;

        index++;
    }

    return -1;
}

void scheduler_init()
{
    klog("sched", "initialising scheduler");
    scheduler_vector = idt_allocate_vector();
    klog("sched", "Allocated scheduler vector 0x%x", scheduler_vector);
    interrupt_table[scheduler_vector] = (void *)((uint64_t)scheduler_isr);
    set_ist(scheduler_vector, 1);
    kernel_process = malloc(sizeof(process_t));
    kernel_process->pagemap = &g_kernel_pagemap;
    // let everyone know we're up and running
    atomic_store(&scheduler_ready, true);
}

void scheduler_isr(__attribute__((unused)) uint32_t num, __attribute__((unused)) cpu_status_t *status)
{
    lapic_timer_stop();
    local_cpu_t *cpu = cpu_get_current();
    atomic_store(&cpu->is_idle, false);
    thread_t *current_thread = get_current_thread();
    int64_t new_index = get_next_thread(cpu->last_run_queue_index); 

    klog("sched", "Getting ready to try to run %d on %d", new_index, cpu->cpu_number);

    klog("sched", "current_thread=%x, new_index=%d, new_thread=%x", current_thread, new_index, scheduler_running_queue[new_index]);

    if (current_thread != 0)
    {
        lock_release(&current_thread->yield_await);

        // the happy case, we're just running the same thread again
        if (new_index == cpu->last_run_queue_index)
        {
            lapic_eoi();
            lapic_timer_oneshot(cpu, scheduler_vector, current_thread->timeslice);
            return;
        }
        // switch context
        current_thread->gs_base = get_kernel_gs_base();
        current_thread->fs_base = get_fs_base();
        current_thread->cr3 = read_cr3();
        fpu_save(current_thread->fpu_storage);
        atomic_store(&current_thread->cpuid, -1);
        lock_release(&current_thread->lock);

        atomic_fetch_sub(&working_cpus, 1);
    }

    if (new_index == -1)
    {
        lapic_eoi();
        set_gs_base(cpu->cpu_number);
        set_kernel_gs_base(cpu->cpu_number);
        cpu->last_run_queue_index = 0;
        atomic_store(&cpu->is_idle, true);
        if (atomic_load(&waiting_event_count) == 0 && atomic_load(&working_cpus) == 0)
        {
            panic("Event heartbeat has flatlined :(");
        }
        scheduler_await();
    }

    atomic_fetch_add(&working_cpus, 1);

    current_thread = scheduler_running_queue[new_index];
    cpu->last_run_queue_index = new_index;

    set_gs_base((uint64_t)current_thread);
    if (current_thread->cpu_state.cs == USER_CODE_SEGMENT)
    {
        set_kernel_gs_base(current_thread->gs_base);
    }
    else
    {
        set_kernel_gs_base((uint64_t)current_thread);
    }
    set_fs_base(current_thread->fs_base);

    cpu->tss.ist3 = current_thread->pf_stack;

    if (read_cr3() != current_thread->cr3)
        write_cr3(current_thread->cr3);

    fpu_restore(current_thread->fpu_storage);

    atomic_store(&current_thread->cpuid, cpu->cpu_number);

    lapic_eoi();
    lapic_timer_oneshot(cpu, scheduler_vector, current_thread->timeslice);

    cpu_status_t* new_cpu_state = &current_thread->cpu_state;
    if (new_cpu_state->cs == USER_CODE_SEGMENT)
    {
        // todo: dispatch a signal
        klog("sched", "Should dispatch signal but not yet implemented");
    }

    asm volatile(
        "movq %0, %%rsp\n"
        "popq %%rax\n"
        "movl %%eax, %%ds\n"
        "popq %%rax\n"
        "movl %%eax, %%es\n"
        "popq %%rax\n"
        "popq %%rbx\n"
        "popq %%rcx\n"
        "popq %%rdx\n"
        "popq %%rsi\n"
        "popq %%rdi\n"
        "popq %%rbp\n"
        "popq %%r8\n"
        "popq %%r9\n"
        "popq %%r10\n"
        "popq %%r11\n"
        "popq %%r12\n"
        "popq %%r13\n"
        "popq %%r14\n"
        "popq %%r15\n"
        "addq $8, %%rsp\n"
        "swapgs\n"
        "iretq\n"
        :
        : "rm" (new_cpu_state)
        : "memory"
        );


    while (1);
}

void scheduler_await()
{
    // disable interrupts - we can't be interrupted while using cpu_get_current
    asm volatile("cli");
    local_cpu_t *local_cpu = cpu_get_current();
    // wait for 20ms
    lapic_timer_oneshot(local_cpu, scheduler_vector, 20000);
    // enable interrupts and run a HLT loop until the interrupt fires
    asm volatile("sti" ::: "memory");
    for (;;)
    {
        asm volatile("hlt" ::: "memory");
    }
}

bool enqueue_thread(thread_t *thread, bool by_signal)
{
    // shortcut for duplicate calls
    if (thread->is_in_queue)
        return true;

    klog("sched", "Enqueueing thread %x", thread);

    atomic_store(&thread->enqueued_by_signal, by_signal);

    for (uint64_t i = 0; i < MAX_THREADS; i++)
    {
        thread_t *expected = NULL;
        if (atomic_compare_exchange_strong(&scheduler_running_queue[i], &expected, thread))
        {
            thread->is_in_queue = true;

            // wake up any idle CPUs
            for (uint64_t i = 0; i < cpu_count; i++)
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

thread_t *new_kernel_thread(void *ip, void *arg, bool autoenqueue)
{
    klog("sched", "Queueing up new kernel thread");
    void *stacks[PROC_MAX_STACKS_PER_THREAD];
    size_t stack_count = 0;
    void *stack_phys = pmm_alloc(STACK_SIZE / PAGE_SIZE);
    stacks[stack_count++] = stack_phys;
    uint64_t stack = ((uint64_t)stack_phys) + STACK_SIZE + HIGHER_HALF;


    klog("sched", "stack=%x", stack);

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

    thread_t *t = malloc(sizeof(thread_t));
    t->process = kernel_process;
    t->cr3 = (uint64_t)kernel_process->pagemap->top_level;
    t->cpu_state = cpu_state;
    t->timeslice = 5000;
    t->cpuid = (uint64_t)-1;
    memcpy(t->stacks, stacks, sizeof(stacks));
    t->fpu_storage = (void *)((uint64_t)pmm_alloc(div_roundup(fpu_storage_size, PAGE_SIZE)) + HIGHER_HALF);
    t->self = t;
    t->gs_base = (uint64_t)t;


    if (autoenqueue)
    {
        if (!enqueue_thread(t, false))
        {
            panic("Kernel could not enqueue a thread, did we hit MAX_THREADS for kernel threads?");
        }
    }

    return t;
}
