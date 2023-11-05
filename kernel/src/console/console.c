// first-party headers
#include <console/console.h>
#include <term/term.h>
#include <klog/klog.h>
#include <interrupt/idt.h>
#include <interrupt/apic.h>
#include <fs/devtmpfs.h>
#include <errors/errno.h>

// standard headers
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

// a lot of the parameters in this file are unused,
// due to the implementation of the resource interface
// which is incomplete.  So we just turn off -Wunused-parameter
// temporarily.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
// the global console device
console_t console;

bool    console_resource_grow  (resource_t* self, void* handle, uint64_t size);
int64_t console_resource_read  (resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int64_t console_resource_write (resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count);
int     console_resource_ioctl (resource_t* self, void* handle, uint64_t request, void* argp);
bool    console_resource_unref (resource_t* self, void* handle);
bool    console_resource_link  (resource_t* self, void* handle);
bool    console_resource_unlink(resource_t* self, void* handle);
void*   console_resource_mmap  (resource_t* self, uint64_t page, int flags);

void console_init()
{
    console.resource.grow     = console_resource_grow  ;
    console.resource.read     = console_resource_read  ;
    console.resource.write    = console_resource_write ;
    console.resource.ioctl    = console_resource_ioctl ;
    console.resource.unref    = console_resource_unref ;
    console.resource.link     = console_resource_link  ;
    console.resource.unlink   = console_resource_unlink;
    console.resource.mmap     = console_resource_mmap  ;

    uint8_t kbd_vector = idt_allocate_vector();
    klog("console", "kbd_vector=%x", kbd_vector);
    io_apic_set_irq_redirect(local_cpus[0]->lapic_id, kbd_vector, 1, true);
    devtmpfs_add_device(&console.resource, "console");
}

bool    console_resource_grow  (resource_t* self, void* handle, uint64_t size)
{
    // it makes no sense to "grow" the console, so just silently fail
    return false;
}

int64_t console_resource_read  (resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count)
{
    // todo: implement keyboard handling
    return -1;
}

int64_t console_resource_write (resource_t* self, void* handle, void* buf, uint64_t loc, uint64_t count)
{
    char* copy = malloc(count + 1);
    memcpy(copy, buf, count);
    copy[count] = '\0';
    term_printf("%s", copy);
    free(copy);
    return (int64_t)count;
}

int     console_resource_ioctl (resource_t* self, void* handle, uint64_t request, void* argp)
{
    // todo: handle ioctl
    return -1;
}

bool    console_resource_unref (resource_t* self, void* handle)
{
    atomic_fetch_sub(&self->refcount, 1);
    return true;
}

bool    console_resource_link  (resource_t* self, void* handle)
{
    atomic_fetch_add(&self->stat.nlink, 1);
    return true;
}

bool    console_resource_unlink(resource_t* self, void* handle)
{
    atomic_fetch_sub(&self->stat.nlink, 1);
    return true;
}

void*   console_resource_mmap  (resource_t* self, uint64_t page, int flags)
{
    // mmap'ing the console makes no sense
    return NULL;
}

#pragma clang diagnostic pop
