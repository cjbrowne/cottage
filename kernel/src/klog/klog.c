/**
 * This is just a very simple ring buffer based logging system,
 * really only meant for internal kernel use.
 */


#include <stddef.h>
#include "klog.h"
#include <term/term.h>

// keep 128K of logs in the ringbuffer at a time (128K == 131072 bytes)
#define KLOG_BUFSIZE 131072

static char klog_buf[KLOG_BUFSIZE];
static size_t log_end = 0;

#define INC_LOGEND ((++log_end == KLOG_BUFSIZE) && (log_end = 0), log_end)

void klog_putc(char c);

void klog(const char* module, const char* msg)
{
    char* cur_head = klog_buf + log_end;
    size_t log_end_start = log_end;
    klog_putc('[');
    while(*module) {

        klog_buf[INC_LOGEND] = *module;
        module++;
    }
    klog_putc(']');
    klog_putc(' ');
    while(*msg) {
        klog_buf[INC_LOGEND] = *msg;
        msg++;
    }
    // users shouldn't need to put newlines in
    klog_putc('\n');

    // write the buffer out to tty0 as we go
    // deal with the nasty case of a split log that's
    // half at the start of the buffer and half at the end
    if (log_end_start > log_end)
    {
        // write the bits from the end of the buffer
        term_write(cur_head, KLOG_BUFSIZE - log_end_start);
        // write the bytes from the start of the buffer
        term_write(klog_buf, log_end);
    }
    else
    {
        term_write(cur_head, log_end - log_end_start);
    }
}

void klog_putc(char c)
{
    klog_buf[INC_LOGEND] = c;
}