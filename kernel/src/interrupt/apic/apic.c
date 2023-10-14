#include "apic.h"
#include <klog/klog.h>

void write_apic_register(uint16_t reg, uint32_t val)
{
    klog("apic", "Unimplemented: write apic register reg=%x val=%d", reg, val);
}
