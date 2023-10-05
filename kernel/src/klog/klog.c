/**
 * This is just a very simple ring buffer based logging system,
 * really only meant for internal kernel use.
 */


#include <stddef.h>
#include "klog.h"

// keep 128K of logs in the ringbuffer at a time
#define KLOG_BUFSIZE 128*1024

char klog_buf[KLOG_BUFSIZE];
size_t log_end;

#define INC_LOGEND (log_end++, (log_end == KLOG_BUFSIZE) && log_end=0))

void klog_putc(char c);

void klog(const char* module, const char* msg)
{
    klog_putc('[');
    while(*module)
        klog_buf[log_end++] = *module;
    klog_putc(']');
    klog_putc(' ');
    while(*msg)
        klog_buf[log_end++] = *msg;
}

void klog_putc(char c)
{
    klog_buf[log_end++] = c;
}