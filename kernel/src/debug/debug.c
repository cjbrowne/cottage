#include "debug.h"
#include <term/term.h>
#include <klog/klog.h>

void hexdump(void* addr, size_t count)
{
    term_printf("Dumping %d bytes from %x:\n", count, addr);
    for(size_t i = 0; i < count; i++)
    {
        uint8_t b = ((uint8_t*)addr)[i];
        if(b < 0x10) term_putc('0'); // simple zero padding
        term_printf("%x", b);
        if(i % 16 == 15) // put a newline every 16 bytes
        term_putc('\n');
    }
    // if there is not a 16-byte aligned number of bytes, put an extra newline
    // at the end so we don't run into the next log line
    if(count % 16 != 0)
        term_putc('\n');
}
