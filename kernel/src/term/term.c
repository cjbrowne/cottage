// currently only support the flanterm backend
// future, maybe more? (unlikely though)

#include "term.h"
#include "klog/klog.h"
#include <flanterm/backends/fb.h>
#include <flanterm/flanterm.h>
#include <mem/kmalloc.h>
#include <stdio.h>
#include <string.h>

static struct flanterm_context *ctx;

void term_putc(const char c);

void term_init(uint32_t *framebuffer, size_t width, size_t height,
               size_t pitch)
{
    ctx = flanterm_fb_simple_init(framebuffer, width, height, pitch);
}

void term_write(const char *string, size_t count)
{
    flanterm_write(ctx, string, count);
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

void term_printf(const char *fmt, ...)
{
    va_list args;
    char *buf = NULL;
    size_t len = vsnprintf(buf, 0, fmt, args);
    buf = kmalloc(len);
    vsnprintf(buf, len, fmt, args);
    term_write(buf, len);
    kfree(buf);
}
