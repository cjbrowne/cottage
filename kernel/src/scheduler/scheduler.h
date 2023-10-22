#pragma once

#include "event.h"
#include <lock/lock.h>
#include <mem/pagemap.h>
#include <stdint.h>

// todo: make configurable at runtime?
#define PROC_MAX_FDS 256
#define PROC_MAX_EVENTS 32
#define PROC_TIMERS 4096
// be careful when tuning these two as they are multiplicative of one another.
// 64 threads, 64 children = 4096+64 total threads per process
#define PROC_MAX_THREADS_PER_PROCESS 64
#define PROC_MAX_CHILD_PROCESSES 64

typedef struct {
    uint64_t cpuid; // which cpu is the thread running on?
    struct thread_t* self;
    // POSIX errno
    uint64_t errno;
    uint64_t kernel_stack;
    uint64_t user_stack;
    // used for implementing syscalls as interrupts
    uint64_t syscall_num;

    // thread id
    uint64_t tid;
    // is the thread currently queued for running?
    bool is_in_queue;
    lock_t lock;

} thread_t;

typedef struct {
    uint64_t pid;
    uint64_t parent_pid;
    pagemap_t* pagemap;
    uint64_t thread_stack_top;
    thread_t* threads[PROC_MAX_THREADS_PER_PROCESS];
    lock_t fds_lock;
    void* fds[PROC_MAX_FDS];
    struct process_t* children[PROC_MAX_CHILD_PROCESSES];
    // anonymous mmap calls will place memory at this location
    uint64_t mmap_anon_non_fixed_base;
    // todo: change to direntry* maybe?
    void* cwd;
    event_t event;
    uint64_t status;
    uint8_t itimers[PROC_TIMERS];
    char* name;
} process_t;


void scheduler_init();
void scheduler_await();

extern _Atomic uint8_t scheduler_vector;
