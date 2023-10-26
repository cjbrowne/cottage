#include "panic.h"
#include <lock/lock.h>
#include <term/term.h>
#include <string.h>
#include <stdbool.h>
#include <serial/serial.h>
#include <stdarg.h>
#include <stdio.h>
#include <cpu/smp.h>
#include <interrupt/apic.h>
#include <interrupt/isr.h>

// Halt and catch fire function.
__attribute__((noreturn)) static void hcf(void)
{
    asm volatile("cli");
    for (;;)
    {
        asm volatile("hlt");
    }
}

extern bool have_malloc;
extern bool have_term;
extern bool have_smp;

lock_t panic_lock;

__attribute__((noreturn)) void panic(const char *msg, ...)
{
    // only allow panicking once by locking and then never unlocking
    lock_acquire(&panic_lock);
    // don't allow interrupts after a kernel panic
    asm volatile("cli");
    va_list args, args2;
    va_copy(args2, args);

    // if SMP is ready, we should abort all _other_ CPUs
    if(have_smp)
    {
        for (size_t i = 0; i < cpu_count; i++)
        {
            if (cpu_get_current()->lapic_id == local_cpus[i]->lapic_id) continue;
            lapic_send_ipi(local_cpus[i]->lapic_id, abort_vector);
            while(!atomic_load(&local_cpus[i]->aborted));
        }
    }

    if (have_term)
    {
        term_write("PANIC!\n", 7);
        va_start(args, msg);
        term_vprintf(msg, args);
        va_end(args);
    }
    
    {
        // always output serial debugging info
        print_serial("PANIC!\r\n");
        char buf[256];
        va_start(args2, msg);
        vsnprintf(buf, 256, msg, args2);
        va_end(args2);
        print_serial(buf);
        print_serial("\r\n");
    }

    hcf();
}
