#pragma once

#include <stdint.h>

#define PIT_DIVIDEND ((uint64_t)1193182)

void pit_set_reload_value(uint16_t val);
uint16_t pit_get_current_count();


