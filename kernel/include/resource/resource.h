#pragma once

#include <scheduler/event.h>
#include <stat/stat.h>

typedef struct resource_s resource_t;

typedef struct resource_s {
    stat_t stat;
    _Atomic int refcount;
    lock_t lock;
    event_t event;
    int status;
    bool can_mmap;

    // function pointers
    bool (*grow)(resource_t* self, void* handle, uint64_t size);
    int64_t (*read)(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
    int64_t (*write)(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
    int (*ioctl)(resource_t* self, void* handle, uint64_t request, void* argp);
    bool (*unref)(resource_t* self, void* handle);
    bool (*link)(resource_t* self, void* handle);
    bool (*unlink)(resource_t* self, void* handle);
    void* (*mmap)(resource_t* self, uint64_t page, int flags);
} resource_t;

uint64_t resource_create_dev_id();
int resource_default_ioctl(void* handle, uint64_t request, void* argp);
