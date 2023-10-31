#pragma once

#include <resource/resource.h>
#include <lock/lock.h>

#include <stdint.h>

typedef struct {
    resource_t resource;
    lock_t rng_lock;
    uint32_t buffer[16];
    uint32_t key[16];
    uint64_t reseed_ctr;
} dev_random_t;

void random_init();
void random_reseed(dev_random_t* self);

bool random_grow(resource_t* self, void* handle, uint64_t size);
int64_t random_read(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int64_t random_write(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int random_ioctl(resource_t* self, void* handle, uint64_t request, void* argp);
bool random_unref(resource_t* self, void* handle);
bool random_link(resource_t* self, void* handle);
bool random_unlink(resource_t* self, void* handle);
void* random_mmap(resource_t* self, uint64_t page, int flags);
