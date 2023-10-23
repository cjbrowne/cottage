#include "panic.h"
#include <lock/lock.h>
#include <term/term.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

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

lock_t panic_lock;

__attribute__((noreturn)) void panic(const char *msg, ...)
{
    lock_acquire(&panic_lock);
    // don't allow interrupts after a kernel panic
    asm volatile("cli");
    va_list args;
    if (have_term)
    {
        term_write("PANIC!\n", 7);
        va_start(args, msg);
        term_vprintf(msg, args);
        va_end(args);
    }
    lock_release(&panic_lock);

    hcf();
}
