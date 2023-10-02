#include "hal.h"
#include <stdio.h>
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>

void HAL_init()
{
    printf("\t\tInitializing GDT...");
    i686_GDT_Initialize();
    printf("Done\r\n");

    printf("\t\tInitializing IDT...");
    i686_IDT_init();
    printf("Done\r\n");

    printf("\t\tInitializing ISR...");
    i686_ISR_init();
    printf("Done\r\n");

    printf("\t\tInitializing IRQ...");
    i686_IRQ_init();
    printf("Done\r\n");
}