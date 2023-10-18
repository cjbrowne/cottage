#include "network.h"
#include <klog/klog.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

static network_device_t** devices = NULL;
static size_t device_count;

void net_register_device(const char *identifier, network_device_t* device)
{
    klog("net", "Registering networking device %s", identifier);
    
    // step 1: grow the device array
    device_count++;
    devices = realloc(devices, device_count * sizeof(network_device_t*));

    // step 2: insert the device into the end of the array
    devices[device_count - 1] = device;
}

// this function is responsible for initialising the network protocol stack,
// and is intended to be run as a task in the OS scheduler.
bool net_init()
{
    if(device_count == 0) return false;

    return true;
}
