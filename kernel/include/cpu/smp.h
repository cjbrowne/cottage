#pragma once

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <macro.h>
#include <limine.h>

#define ABORT_STACK_SIZE 128

typedef struct {
    RESERVE_BYTES(4);
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    RESERVE_BYTES(8);
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    RESERVE_BYTES(8);
    uint32_t iopb;
} __attribute__((packed)) task_state_segment_t;

typedef struct {
    uint64_t cpu_number;
    uint64_t zero;
    task_state_segment_t tss;
    uint32_t lapic_id;
    uint64_t lapic_timer_freq;
    _Atomic uint64_t online;
    _Atomic bool is_idle;
    int64_t last_run_queue_index;
    uint64_t abort_stack[ABORT_STACK_SIZE];
    _Atomic bool aborted;
} local_cpu_t;


void smp_init(struct limine_smp_response* smp_response);
void cpu_init(struct limine_smp_info* smp_info);
local_cpu_t* cpu_get_current();

extern local_cpu_t** local_cpus;
extern uint32_t bsp_lapic_id;
extern uint64_t cpu_count;
extern bool have_smp;


