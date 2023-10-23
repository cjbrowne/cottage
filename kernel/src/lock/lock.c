#include "lock.h"
#include <stdatomic.h>
#include <klog/klog.h>
#include <panic.h>

void lock_acquire(lock_t* lock)
{
    uint64_t caller = (uint64_t) __builtin_return_address(0);

    // I believe this magic number is just "a big number",
    // that gives us as much chance as possible at getting the lock in time
    // it should probably be abstracted out into some kind of compile time const,
    // or configuration variable, but I can't be bothered right now.
    for (uint64_t i = 0; i < 50000000; i++)
    {
        if (lock_test_and_acquire(lock))
        {
            lock->caller = caller;
            return;
        }
        asm volatile ( "pause" ::: "memory" );
    }

    // todo: implement tracing to get some symbolic names going and easier 
    // debugging of deadlock conditions
    klog("lock", "Lock address: %llx", (void*)lock);
    klog("lock", "Current caller: %llx", caller);
    klog("lock", "Last caller: %llx", lock->caller);

    panic("Deadlock detected");
}

void lock_release(lock_t* lock)
{
    atomic_store(&lock->is_locked, false);
}

bool lock_test_and_acquire(lock_t* lock)
{
    uint64_t caller = (uint64_t) __builtin_return_address(0);
    
    bool expected = false;
    bool new_value = true;
    
    if(atomic_compare_exchange_strong(&lock->is_locked, &expected, new_value))
    {
        lock->caller = caller;
    }

    return lock->is_locked;
}
