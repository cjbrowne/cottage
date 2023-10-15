#include <klog/klog.h>
#include <string.h>
#include <mem/malloc.h>

#include <lai/host.h>
#include <acpispec/tables.h>

#include <panic.h>
#include <acpi/acpi.h>

/* these functions are strongly linked,
    so are 100% needed for the thing to even compile
*/
void *laihost_malloc(size_t len)
{
    return malloc(len);
}

void *laihost_realloc(void *ptr, size_t newsize, size_t oldsize)
{
    if (newsize == oldsize)
    {
        return ptr;
    }
    else if (newsize < oldsize)
    {
        return ptr; // we just waste some RAM in this case
    }
    else
    { // if (newsize > oldsize)
        // allocate a new range that's big enough for the new size
        void *newptr = malloc(newsize);

        // copy anything that was already present
        memcpy(newptr, ptr, oldsize);

        // free the old space
        free(ptr);

        return newptr;
    }
}

void laihost_free(void *ptr, __attribute__((unused)) size_t len)
{
    free(ptr);
}

/*
 * These functions are very important, but weakly linked,
 * so might or might not work as expected
 * */

void laihost_log(int lvl, const char *msg)
{
    switch (lvl)
    {
    case LAI_DEBUG_LOG:
        klog("lai", "[DEBUG] %s", msg);
        break;
    case LAI_WARN_LOG:
        klog("lai", "[WARN] %s", msg);
        break;
    }
}

__attribute__((noreturn)) void laihost_panic(const char *msg)
{
    panic(msg);
}

/**
 * Table functions
 */

void *laihost_scan(const char *sig, size_t index)
{
    if (memcmp(sig, "DSDT", 4) == 0)
    {
        // todo: detect whether we're using XDST or RDST, and use the correct
        // field (x_dsdt or dsdt, respectively) -- for now we just assume XDST
        // because RDST is a pain in the arse
        return (void *)((acpi_fadt_t *)acpi_find_table("FACP", 0))->x_dsdt;
    }
    else
    {
        return acpi_find_table((char *)sig, index);
    }
}
