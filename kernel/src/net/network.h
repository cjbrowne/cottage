#pragma once

#include <stdbool.h>
#include <stdint.h>

// network driver interface
typedef struct {
    // exposed functions
    void    (*init)();
    bool    (*probe)();
} network_driver;

bool net_init();