#pragma once

#include <cpu/smp.h>
#include <stdint.h>

void lapic_enable(uint16_t spurious_vector);
uint32_t lapic_read(uint32_t reg);
void lapic_write (uint32_t reg, uint32_t val);
void lapic_timer_stop();
void lapic_timer_calibrate();
void lapic_timer_oneshot(local_cpu_t* local_cpu, uint8_t vector, uint64_t micros);
void lapic_eoi();

// todo: move IOAPIC functions to a different file?
void io_apic_set_irq_redirect(uint32_t lapic_id, uint8_t vector, uint8_t irq, bool status);

