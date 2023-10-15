#pragma once

#include <stdint.h>
#include <stdbool.h>

uint64_t* get_next_level(uint64_t* current_level, uint64_t index, bool allocate);
void vmm_init(uint64_t kernel_base_physical, uint64_t kernel_base_virtual);
