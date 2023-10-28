#pragma once

#include <stdarg.h>

void syscall_klog(const char* msg, ...);
void vklog(const char* module, const char* msg, va_list args);
void klog(const char *module, const char *msg, ...);
