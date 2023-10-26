#pragma once

#include <interrupt/idt.h>
#include <stddef.h>

// prints <count> bytes from <addr> in an easy to read format
void hexdump(void* addr, size_t count);
void dump_cpu_state(const cpu_status_t* cpu_state);
