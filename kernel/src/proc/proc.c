#include <proc/proc.h>
#include <panic.h>

_Atomic(process_t*) processes[PROC_MAX_PROCESSES];

thread_t* get_current_thread()
{
    thread_t* ret = NULL;

    asm volatile (
        "movq %%gs:[8], %0"
        : "=r" (ret)
    );

    return ret;
}

uint64_t proc_allocate_pid(process_t* process)
{
    for (uint64_t i = 0; i < PROC_MAX_PROCESSES; i++)
    {
        process_t* null = NULL;
        if (atomic_compare_exchange_strong(&processes[i], &null, process))
            return i;
    }
    // todo: maybe set errno instead and have the caller check errno and
    //  handle it gracefully instead?
    panic("PID exhaustion!");
}

