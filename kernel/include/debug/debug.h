#pragma once

#include <stddef.h>

// prints <count> bytes from <addr> in an easy to read format
void hexdump(void* addr, size_t count);
