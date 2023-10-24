// currently only support the flanterm backend
// future, maybe more? (unlikely though)

#include "term.h"
#include <lock/lock.h>
#include <flanterm/backends/fb.h>
#include <flanterm/flanterm.h>
#include <mem/malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <macro.h>
#include <cpu/smp.h>
#include <serial/serial.h>

extern bool have_term;

static struct flanterm_context *ctx;

static lock_t term_lock;

void term_putc(const char c);

void term_init(uint32_t *framebuffer, size_t width, size_t height,
               size_t pitch)
{
    ctx = flanterm_fb_simple_init(framebuffer, width, height, pitch);
    ctx->autoflush = false; // disable autoflushing for SMP reasons
}

void term_write(const char *string, size_t count)
{
    have_term = false; // stops a PANIC loop
    // term writes must be locked since async writes will corrupt the framebuffer
    lock_acquire(&term_lock);
    flanterm_write(ctx, string, count);
    print_serial(string);
    print_serial("\r"); // add a carriage return after each write on serial
    if(have_smp)
    {
        asm volatile("cli" ::: "memory");
        uint64_t cpuid = cpu_get_current()->cpu_number;
        if(cpuid == 0)
            ctx->double_buffer_flush(ctx);
        asm volatile("sti" ::: "memory");
    }
    else
    {
        ctx->double_buffer_flush(ctx);
    }
    lock_release(&term_lock);
    have_term = true;
}

void term_putc(const char c) { term_write(&c, 1); }

// todo: move this to string.h/string.c
// unsafe strlen function, counts until it hits a NULL byte with no limit
// enforced
size_t strlen(const char *str)
{
    const char *s = str;
    while (*(s++))
        ;
    return s - str;
}

void term_vprintf(const char* fmt, va_list args)
{
    lock_acquire(&term_lock);
    char buf[256] = {0};
    vsnprintf(buf, 256, fmt, args);
    term_write(buf, 256);
    lock_release(&term_lock);
}

void term_printf(const char *fmt, ...)
{
    lock_acquire(&term_lock);
    va_list args;
    va_start(args, fmt);
    term_vprintf(fmt, args);
    va_end(args);
    lock_release(&term_lock);
}
