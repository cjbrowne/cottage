#pragma once

#include <stdarg.h>

void syscall_klog(const char* msg, ...);
void vklog(const char* module, const char* msg, va_list args);
void klog(const char *module, const char *msg, ...);

#ifdef COTTAGE_DEBUG
// use this to add debug logs, so that they can be distinguished from actual logs
// and excluded from production builds
void klog_debug(const char* module, const char* msg, ...);
#else
#define klog_debug(...)
#endif


