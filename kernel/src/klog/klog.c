/**
 * This is just a very simple ring buffer based logging system,
 * really only meant for internal kernel use.
 */

#include "klog.h"
#include <math/si.h>
#include <mem/malloc.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <term/term.h>

// keep 128K of logs in the ringbuffer at a time
#define KLOG_BUFSIZE KiB(128)

static char klog_buf[KLOG_BUFSIZE];
static size_t log_end = 0;

// couple of kernel booleans to help us track which features have been
// bootstrapped so far
extern bool have_term;
extern bool have_malloc;

void klog_putc(char c);

void klog(const char *module, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t log_end_start = log_end;
    klog_putc('[');
    while (*module)
    {
        klog_buf[log_end++] = *module;
        if (log_end == KLOG_BUFSIZE)
            log_end = 0;
        module++;
    }
    klog_putc(']');
    klog_putc(' ');

    int max_chars = KLOG_BUFSIZE - log_end - 1;
    int chars_written = vsnprintf(klog_buf + log_end, max_chars, fmt, args);

    if (chars_written > max_chars)
    {
        // output will simply get truncated if we don't have malloc yet,
        // this allows us to use klog calls earlier in the boot process
        // while not trying to call an uninitialized malloc.
        if (have_malloc)
        {
            char *line_buf = malloc(chars_written);
            vsnprintf(line_buf, chars_written, fmt, args);
            memcpy(klog_buf, line_buf + max_chars, max_chars - chars_written);
            log_end = max_chars - chars_written;
            free(line_buf);
        }
        else
        {
            // we wrote up to the end of the buffer, and then stopped because
            // we don't have malloc yet (truncating the log entry)
            log_end = 0;
        }
    }
    else
    {
        log_end += chars_written;
    }

    // users shouldn't need to put their own newlines in
    klog_putc('\n');

    // if we don't have a terminal, hopefully by the time we do have one
    // we can start printing to it
    if (have_term)
    {
        // write the buffer out to tty0 as we go
        // deal with the nasty case of a split log that's
        // half at the start of the buffer and half at the end
        if (log_end_start > log_end)
        {
            // write the bits from the end of the buffer
            term_write(klog_buf + log_end_start, KLOG_BUFSIZE - log_end_start);
            // write the bytes from the start of the buffer
            term_write(klog_buf, log_end);
        }
        else
        {
            //      term_printf("Printing");
            term_write(klog_buf + log_end_start, (log_end - log_end_start));
        }
    }
    va_end(args);
}

void klog_putc(char c)
{
    klog_buf[log_end++] = c;
    if (log_end == KLOG_BUFSIZE)
        log_end = 0;
}
