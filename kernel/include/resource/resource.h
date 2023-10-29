#pragma once

#include <scheduler/event.h>
#include <stat/stat.h>

typedef struct {
    stat_t stat;
    _Atomic int refcount;
    lock_t lock;
    event_t event;
    int status;
    bool can_mmap;
} resource_t;

uint64_t resource_create_dev_id();
