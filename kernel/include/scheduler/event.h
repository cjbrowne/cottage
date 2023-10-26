#pragma once

#include <lock/lock.h>
#include <stdint.h>

#define EVT_MAX_LISTENERS 32

typedef struct {
    void* thread;
    uint64_t which;
} event_listener_t;

typedef struct {
    lock_t lock;
    uint64_t pending;
    uint64_t listeners_index;
    event_listener_t listeners[EVT_MAX_LISTENERS];
} event_t;

uint64_t event_trigger(event_t* event, bool drop);

extern _Atomic uint64_t waiting_event_count;
