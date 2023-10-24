#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    _Atomic bool is_locked;
    uint64_t caller;
} lock_t;

void lock_acquire(lock_t* lock);
void lock_release(lock_t* lock);
bool lock_test_and_acquire(lock_t* lock);
