#pragma once

#include <stdbool.h>

typedef struct {
    bool present;
    // the entrypoint where the driver was loaded into memory
    void* start;
    char* filepath;
} driver_t;

void drivers_init();
