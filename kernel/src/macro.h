#pragma once

#include <panic.h>

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)

#define CHECKPOINT panic("reached checkpoint in " __FILE__ ":" STRINGIFY(__LINE__));

#define RESERVE_BITS(x) uint64_t :x
#define RESERVE_BYTES(x) RESERVE_BITS(x * 8)


