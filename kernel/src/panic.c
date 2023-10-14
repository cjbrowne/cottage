#include "panic.h"
#include <term/term.h>
#include <string.h>
#include <stdbool.h>

// Halt and catch fire function.
__attribute__((noreturn)) static void hcf(void)
{
    asm("cli");
    for (;;)
    {
        asm("hlt");
    }
}

extern bool have_term;

__attribute__((noreturn)) void panic(const char *msg)
{
    if (have_term)
    {
        term_write("PANIC!\n", 7);
        term_write(msg, strlen(msg));
    }

    hcf();
}
