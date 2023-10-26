#include <debug/debug.h>
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

void dump_cpu_state(const cpu_status_t* status)
{
    const char* module = "debug";
    klog(module, "CPU Status Dump:");
    klog(module, "  DS: %x  ES: %x", status->ds, status->es);
    klog(module, " RAX: %x RBX: %x", status->rax, status->rbx);
    klog(module, " RCX: %x RDX: %x", status->rcx, status->rdx);
    klog(module, " RSI: %x RDI: %x", status->rsi, status->rdi);
    klog(module, " RBP: %x  R8: %x", status->rbp, status->r8);
    klog(module, "  R9: %x R10: %x", status->r9, status->r10);
    klog(module, "R11: %x R12: %x", status->r11, status->r12);
    klog(module, "R13: %x R14: %x", status->r13, status->r14);
    klog(module, "R15: %x", status->r15);

    klog(module, "Error Code: %x", status->error_code);

    klog(module, "RIP: %x  CS: %x", status->rip, status->cs);
    klog(module, "RFLAGS: %x", status->rflags);
    klog(module, "RSP: %x  SS: %x", status->rsp, status->ss);
}
