#pragma once

// the very first syscall, a simple "klog" wrapper
#define SYS_KLOG 0

void syscall_entry();
void syscall_init();
