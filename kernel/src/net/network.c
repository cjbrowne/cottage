#include "rtl8139/rtl8139.h"
#include "network.h"
#include <klog/klog.h>

bool net_init()
{
    network_driver *driver = rtl8139_get_driver();
    if (!driver->probe()) {
        klog("net", "network driver not present");
        return false;
    }
    return true;
}