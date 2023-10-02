#include <stdint.h>
#include <stdio.h>
#include "memory.h"
#include <hal/hal.h>
#include <arch/i686/irq.h>
#include <boot/bootparams.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void timer(registers* regs)
{
    printf(".");
}

_Noreturn void __attribute__((section(".entry"))) start(boot_params* bp)
{
    memset(&__bss_start, 0, (&__end) - (&__bss_start));
    clrscr();
    printf("=== Kernel Loaded ===\r\n");
    printf("Kernel Version 0.0.1b (snapshot release)\r\n");
    printf("Initializing HAL...\r\n");
    HAL_init();
    printf("HAL Initialized\r\n");

    printf("Boot device: %x\r\n", bp->boot_device);
    printf("Memory region count: %d\r\n", bp->memory.region_count);
    for (int i = 0; i < bp->memory.region_count; i++)
    {
        printf("MEM: start=0x%llx length=0x%llx type=%x\r\n",
                  bp->memory.regions[i].begin,
                  bp->memory.regions[i].length,
                  bp->memory.regions[i].type);
    }

    i686_IRQ_RegisterHandler(0, timer);
end:
    for(;;);
}