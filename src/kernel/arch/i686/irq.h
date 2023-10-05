#pragma once
#include "isr.h"

typedef void (*IRQHandler)(registers* regs);

void i686_IRQ_init();
void i686_IRQ_RegisterHandler(int irq, IRQHandler handler);
