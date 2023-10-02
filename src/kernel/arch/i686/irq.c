#include "isr.h"
#include "irq.h"
#include "i8259.h"
#include "io.h"
#include <stddef.h>
#include <stdio.h>
#include <util/arrays.h>

#define PIC_REMAP_OFFSET 0x20

IRQHandler g_IRQHandlers[16];
static const PICDriver* g_Driver = NULL;

void i686_IRQ_Handler(registers* regs)
{
    int irq = regs->interrupt_number - PIC_REMAP_OFFSET;

    if (g_IRQHandlers[irq] != NULL) {
        g_IRQHandlers[irq](regs);
    } else {
        printf("Unhandled IRQ %d\n", irq);
    }

    // send EOI
    g_Driver->sendEndOfInterrupt(irq);
}

void i686_IRQ_init()
{
    const PICDriver* drivers[] = {
            i8259_GetDriver(),
    };

    for(int i = 0; i < SIZE(drivers); i++)
    {
        if(drivers[i]->probe()) {
            g_Driver = drivers[i];
        }
    }

    if (g_Driver == NULL)
    {
        printf("Warning: No PIC found!\r\n");
        return;
    }

    printf("Found %s PIC driver\r\n", g_Driver->name);
    g_Driver->init(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

    // register ISR handler for each of the 16 irq lines
    for(int i = 0; i < 16; i++)
        i686_ISR_RegisterHandler(PIC_REMAP_OFFSET + i, i686_IRQ_Handler);

    // enable interrupts
    i686_EnableInterrupts();
}

void i686_IRQ_RegisterHandler(int irq, IRQHandler handler)
{
    g_IRQHandlers[irq] = handler;
}
