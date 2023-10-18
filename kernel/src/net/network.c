#include "network.h"
#include <klog/klog.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

static network_device_descriptor_t* devices = NULL;
static size_t device_count;

void net_register_device(const char *identifier, network_device_t* device)
{
    klog("net", "Registering networking device %s", identifier);
    
    // step 1: grow the device array
    device_count++;
    devices = realloc(devices, device_count * sizeof(network_device_descriptor_t));

    // step 2: insert the device into the end of the array
    devices[device_count - 1] = (network_device_descriptor_t) {
        .device = device,
        .identifier = identifier
    };
}

// very low-level call, simply writes <len> bytes to the network device's
// send buffer.  Used by higher level APIs to implement protocol writes.
// it is not expected for typical users to be using this function directly.
// returns the number of bytes written (if <= len, it means the write was
// truncated, possibly due to lack of buffer space)
size_t net_write(const char* devid, uint8_t* ptr, size_t len)
{
    for(size_t i = 0; i < device_count; i++)
    {
        if(strcmp(devices[i].identifier, devid) == 0)
        {
            if(devices[i].device->send_buf_len + len > devices[i].device->send_buf_max)
            {
                // only send as many bytes as we actually can
                len = (devices[i].device->send_buf_max - devices[i].device->send_buf_len);
            }
            memcpy(devices[i].device->send_buf, ptr, len);
            devices[i].device->send_buf_len += len;
            break;
        }
    }
    return len;
}

// read at most <len> bytes from the device.
// returns the number of bytes actually read
size_t net_read(const char* devid, uint8_t* buf, size_t len)
{
    size_t bytes_read = 0;
    for(size_t i = 0; i < device_count; i++)
    {
        if(strcmp(devices[i].identifier, devid) == 0)
        {
            network_device_t* dev = devices[i].device;
            for (size_t i = 0; dev->recv_buf_len > 0 && i < len; i++)
            {
                memcpy(buf, *dev->recv_buf_read_ptr, 1);
                (*dev->recv_buf_read_ptr)++;
                // if we hit the end, wrap the pointer back around
                if(*dev->recv_buf_read_ptr - dev->recv_buf == dev->recv_buf_len)
                {
                    *dev->recv_buf_read_ptr = dev->recv_buf;
                }
                dev->recv_buf_len--;
                bytes_read++;
            }
        }
    }
    return bytes_read;
}

// this function is responsible for initialising the network protocol stack,
// and is intended to be run as a task in the OS scheduler.
bool net_init()
{
    if(device_count == 0) return false;

    return true;
}
