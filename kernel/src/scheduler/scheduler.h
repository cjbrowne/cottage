#pragma once

#include <stdint.h>

void scheduler_init();
void scheduler_await();

extern uint8_t scheduler_vector;
