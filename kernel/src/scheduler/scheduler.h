#pragma once

#include "event.h"
#include <stdatomic.h>
#include <lock/lock.h>
#include <mem/pagemap.h>
#include <stdint.h>
#include <interrupt/idt/idt.h>

// todo: make configurable at runtime?
#define PROC_MAX_FDS 256
#define PROC_MAX_EVENTS 32
#define PROC_TIMERS 4096
// be careful when tuning these two as they are multiplicative of one another.
// 64 threads, 64 children = 4096+64 total threads per process
#define PROC_MAX_THREADS_PER_PROCESS 64
#define PROC_MAX_CHILD_PROCESSES 64

// some per-thread maximums
#define PROC_MAX_STACKS_PER_THREAD 64
#define PROC_MAX_SIGNAL_FDS_PER_THREAD 64

typedef struct process_s process_t;
typedef struct thread_s thread_t;

typedef struct {
    void* sa_sigaction;
    uint64_t sa_mask;
    uint64_t flags;
} sig_action_t;

typedef struct thread_s {
    uint64_t cpuid; // which cpu is the thread running on?
    thread_t* self;
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
    process_t* process;
    cpu_status_t cpu_state;
    uint64_t gs_base;
    uint64_t fs_base;
    uint64_t pf_stack;
    uint64_t cr3;
    void* fpu_storage;
    lock_t yield_await;
    uint64_t timeslice;
    uint64_t which_event;
    void* exit_value;
    event_t exited;
    uint64_t sigentry;
    sig_action_t sigactions[256];
    uint64_t pending_signals;
    uint64_t masked_signals;
    _Atomic bool enqueued_by_signal;
    void* stacks[PROC_MAX_STACKS_PER_THREAD];
    lock_t signalfds_lock;
    void* signalfds[PROC_MAX_SIGNAL_FDS_PER_THREAD];
    event_t attached_events[PROC_MAX_EVENTS];
    uint64_t attached_events_index;
} thread_t;

typedef struct process_s {
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
thread_t* new_kernel_thread(void* ip, void* arg, bool autoenqueue);
bool enqueue_thread(thread_t* thread, bool by_signal);

extern _Atomic uint8_t scheduler_vector;
