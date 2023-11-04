/*

    This file is what creates the streaming devices:
    /dev/null
    /dev/zero
    /dev/full

*/

#pragma clang diagnostic push
// ignore unused parameters, this file will be full of them due to implementing
// of an interface that expects us to do more than we actually want to do
#pragma clang diagnostic ignored "-Wunused-parameter"

// first-party headers
#include <streams/streams.h>
#include <resource/resource.h>
#include <panic.h>
#include <mem/pmm.h>
#include <fs/devtmpfs.h>
#include <errors/errno.h>

// standard headers
#include <stdint.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

bool devnull_grow(resource_t* self, void* handle, uint64_t size);
int64_t devnull_read(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int64_t devnull_write(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int devnull_ioctl(resource_t* self, void* handle, uint64_t request, void* argp);
bool devnull_unref(resource_t* self, void* handle);
bool devnull_link(resource_t* self, void* handle);
bool devnull_unlink(resource_t* self, void* handle);
void* devnull_mmap(resource_t* self, uint64_t page, int flags);

resource_t devnull;

bool devzero_grow(resource_t* self, void* handle, uint64_t size);
int64_t devzero_read(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int64_t devzero_write(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int devzero_ioctl(resource_t* self, void* handle, uint64_t request, void* argp);
bool devzero_unref(resource_t* self, void* handle);
bool devzero_link(resource_t* self, void* handle);
bool devzero_unlink(resource_t* self, void* handle);
void* devzero_mmap(resource_t* self, uint64_t page, int flags);

resource_t devzero;

bool devfull_grow(resource_t* self, void* handle, uint64_t size);
int64_t devfull_read(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int64_t devfull_write(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int devfull_ioctl(resource_t* self, void* handle, uint64_t request, void* argp);
bool devfull_unref(resource_t* self, void* handle);
bool devfull_link(resource_t* self, void* handle);
bool devfull_unlink(resource_t* self, void* handle);
void* devfull_mmap(resource_t* self, uint64_t page, int flags);

resource_t devfull;

void streams_init()
{
    // devnull init
    devnull.grow = devnull_grow;
    devnull.read = devnull_read;
    devnull.write = devnull_write;
    devnull.ioctl = devnull_ioctl;
    devnull.unref = devnull_unref;
    devnull.link = devnull_link;
    devnull.unlink = devnull_unlink;
    devnull.mmap = devnull_mmap;
    devnull.stat.size = 0;
    devnull.stat.blocks = 0;
    devnull.stat.block_size = 4096;
    devnull.stat.rdev = resource_create_dev_id();
    devnull.stat.mode = 0666 | STAT_IFCHR;

    devtmpfs_add_device(&devnull, "null");

    // devzero init
    devzero.grow = devzero_grow;
    devzero.read = devzero_read;
    devzero.write = devzero_write;
    devzero.ioctl = devzero_ioctl;
    devzero.unref = devzero_unref;
    devzero.link = devzero_link;
    devzero.unlink = devzero_unlink;
    devzero.mmap = devzero_mmap;
    devzero.stat.size = 0;
    devzero.stat.blocks = 0;
    devzero.stat.block_size = 4096;
    devzero.stat.rdev = resource_create_dev_id();
    devzero.stat.mode = 0666 | STAT_IFCHR;
    devzero.can_mmap = true;

    devtmpfs_add_device(&devzero, "zero");
    
    // devfull init
    devfull.grow = devfull_grow;
    devfull.read = devfull_read;
    devfull.write = devfull_write;
    devfull.ioctl = devfull_ioctl;
    devfull.unref = devfull_unref;
    devfull.link = devfull_link;
    devfull.unlink = devfull_unlink;
    devfull.mmap = devfull_mmap;
    devfull.can_mmap = true;

    devtmpfs_add_device(&devfull, "full");
}

bool devnull_grow(resource_t* self, void* handle, uint64_t size)
{
    return true;
}

int64_t devnull_read(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count)
{
	return 0;
}

int64_t devnull_write(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count)
{
    // writes against /dev/null should always succeed, but do nothing
    return count;
}

int devnull_ioctl(resource_t* self, void* handle, uint64_t request, void* argp)
{
    return resource_default_ioctl(handle, request, argp);
}

bool devnull_unref(resource_t* self, void* handle)
{
    atomic_fetch_sub(&self->refcount, 1);
    return true;
}

bool devnull_link(resource_t* self, void* handle)
{
    atomic_fetch_add(&self->stat.nlink, 1);
    return true;
}

bool devnull_unlink(resource_t* self, void* handle)
{
    atomic_fetch_sub(&self->stat.nlink, 1);
    return true;
}

void* devnull_mmap(resource_t* self, uint64_t page, int flags)
{
    // todo: should this really be a panic?
    panic("Cannot mmap /dev/null");
}


bool devzero_grow(resource_t* self, void* handle, uint64_t size)
{
    return true;
}

int64_t devzero_read(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count)
{
    memset(buf, 0, count);
    return count;
}

int64_t devzero_write(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count)
{
    return count;
}

int devzero_ioctl(resource_t* self, void* handle, uint64_t request, void* argp)
{
    return resource_default_ioctl(handle, request, argp);
}

bool devzero_unref(resource_t* self, void* handle)
{
    atomic_fetch_sub(&self->refcount, 1);
    return true;
}

bool devzero_link(resource_t* self, void* handle)
{
	atomic_fetch_add(&self->stat.nlink, 1);
    return true;
}

bool devzero_unlink(resource_t* self, void* handle)
{
	atomic_fetch_sub(&self->stat.nlink, 1);
    return true;
}

void* devzero_mmap(resource_t* self, uint64_t page, int flags)
{
    return pmm_alloc(1);
}


bool devfull_grow(resource_t* self, void* handle, uint64_t size)
{
	return true;
}

int64_t devfull_read(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count)
{
    memset(buf, 0, count);
    return count;
}

int64_t devfull_write(resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count)
{
    set_errno(ENOSPC);
    return -1;
}

int devfull_ioctl(resource_t* self, void* handle, uint64_t request, void* argp)
{
    return resource_default_ioctl(handle, request, argp);
}

bool devfull_unref(resource_t* self, void* handle)
{
    atomic_fetch_sub(&self->refcount, 1);
    return true;
}

bool devfull_link(resource_t* self, void* handle)
{
    atomic_fetch_add(&self->refcount, 1);
    return true;
}

bool devfull_unlink(resource_t* self, void* handle)
{
    atomic_fetch_sub(&self->refcount, 1);
    return true;
}

void* devfull_mmap(resource_t* self, uint64_t page, int flags)
{
    return pmm_alloc(1);
}

#pragma clang diagnostic pop
