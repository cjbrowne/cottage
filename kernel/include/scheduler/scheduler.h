#pragma once

#include <proc/proc.h>


void scheduler_init();
void scheduler_await();
void scheduler_dequeue_and_die();
bool scheduler_dequeue_thread(thread_t* thread);
void scheduler_yield(bool save_context);
thread_t* new_kernel_thread(void* ip, void* arg, bool autoenqueue);
bool enqueue_thread(thread_t* thread, bool by_signal);

extern _Atomic uint8_t scheduler_vector;
extern _Atomic bool scheduler_ready;
