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

static struct flanterm_context *ctx;

static lock_t term_lock;

void term_putc(const char c);

void term_init(uint32_t *framebuffer, size_t width, size_t height,
               size_t pitch)
{
    ctx = flanterm_fb_simple_init(framebuffer, width, height, pitch);
}

void term_write(const char *string, size_t count)
{
    // term writes must be locked since async writes will corrupt the framebuffer
    lock_acquire(&term_lock);
    flanterm_write(ctx, string, count);
    lock_release(&term_lock);
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
    // just allocate a big-ass buffer, fuck it.
    char *buf = malloc(512);
    vsnprintf(buf, 512, fmt, args);
    term_write(buf, 512);
    free(buf);
}

void term_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    term_vprintf(fmt, args);
    va_end(args);
}
