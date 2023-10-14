#include "net/network.h"

network_driver rtl8139_driver;

// used for PCI lookup
static const __attribute__((unused)) uint16_t vendor_id = 0x10Ec;
static const __attribute__((unused)) uint16_t device_id = 0x8139;

bool rtl8139_probe();
void rtl8139_init();

network_driver *rtl8139_get_driver()
{
    rtl8139_driver.probe = rtl8139_probe;
    rtl8139_driver.init = rtl8139_init;
    return &rtl8139_driver;
}

bool rtl8139_probe()
{

    return false;
}

void rtl8139_init()
{
}
