#include "apic.h"
#include <klog/klog.h>
#include <cpu/msr.h>
#include <mem/pmm.h>
#include <cpu/kio.h>

static uint64_t lapic_base = 0;

uint32_t lapic_read(uint32_t reg)
{
   if (lapic_base == 0)
   {
        lapic_base = (rdmsr(0x1b) & 0xfffff000) + HIGHER_HALF;
   } 
   return mmin((void*)(lapic_base + reg));
}

void lapic_write (uint32_t reg, uint32_t val)
{
   if (lapic_base == 0)
   {
        lapic_base = (rdmsr(0x1b) & 0xfffff000) + HIGHER_HALF;
   } 
   return mmout((void*)(lapic_base + reg), val);
}
