#pragma once

#include <stdint.h>

typedef struct {
    uint64_t begin, length;
    uint32_t type;
    uint32_t acpi;
} memory_region;

typedef struct {
    int region_count;
    memory_region* regions;
} memory_info;

typedef struct {
    memory_info memory;
    uint8_t boot_device;
} boot_params;