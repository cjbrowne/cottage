#include <sys/syscall.h>
#include <stdarg.h>

void klog(__attribute__((unused)) char* fmt, ...)
{
    va_list ptr;
    // issue SYSCALL assembly instruction, with SYS_KLOG as the operand
    __asm__ volatile
    (
        "syscall"
        : 
        //                 EDI      RSI       RDX
        : "rax"(SYS_KLOG), "D"(fmt), "S"(ptr), "d"(0)
        : "rcx", "r11", "memory"
    );
}

int main(void)
{
    klog("hello, syscalls!");
    // hang
    for(;;);
    return 0;
}
