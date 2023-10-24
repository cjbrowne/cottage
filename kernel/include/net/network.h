#pragma once

#include <stdint.h>
#include <stdbool.h>

// flags
// is the device enabled?
#define NET_DEV_STATUS_ENABLE (1 << 0)
// is the device linked
#define NET_DEV_STATUS_LINK (1 << 1)
// is the device link ready?
#define NET_DEV_STATUS_LINK_READY (1 << 2)

typedef struct
{
    // the device's internal name (e.g "E1000")
    const char *name;
    // write bytes here to send them to the network
    uint8_t *send_buf;

    // write >0 to this file and the network device will send it
    uint32_t send_buf_len;

    // the size (in bytes) of the send buffer
    const uint32_t send_buf_max;

    // read bytes from here when they become available
    uint8_t *recv_buf;
    // when reading, update this to point to the byte after the last byte you read,
    // wrapping around to 0 if necessary
    // do not update or use this for writes, use your own internal write pointer
    uint8_t **recv_buf_read_ptr;
    // if >0, there are bytes waiting to be read from recv_buf
    // when reading, decrement for every byte you read
    // when writing, increment for every byte you write
    uint32_t recv_buf_len;

    // the size (in bytes) of the receive buffer
    const uint32_t recv_buf_max;

    // various control flags, used to control the device
    const uint8_t flags;
} network_device_t;

typedef struct
{
    const char* identifier;
    network_device_t* device;
} network_device_descriptor_t;

// registers device <device> with identifier <identifier>,
// identifier will be used to name the /dev file, so must be a
// compliant filename without any special characters
void net_register_device(const char *identifier, network_device_t* device);

// initialises the network, if there are no devices registered,
// it will return false.
bool net_init();
