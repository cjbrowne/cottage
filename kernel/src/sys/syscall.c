#include "syscall.h"
#include <panic.h>
#include <klog/klog.h>

#define SYSCALL_NUM_ENTRIES 1

void* syscall_table[SYSCALL_NUM_ENTRIES];

void syscall_entry()
{
    panic("Syscalls not yet implemented");
}

void syscall_init()
{
    syscall_table[0] = syscall_klog;
}
