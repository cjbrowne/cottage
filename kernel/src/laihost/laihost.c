#include <klog/klog.h>
#include <string.h>
#include <mem/kmalloc.h>
#include <lai/host.h>
#include <panic.h>

/* these functions are strongly linked,
    so are 100% needed for the thing to even compile
*/
void *laihost_malloc(size_t len)
{
  return kmalloc(len);
}

void *laihost_realloc(void *ptr, size_t newsize, size_t oldsize)
{
  if(newsize == oldsize) {
    return ptr;
  } else if(newsize < oldsize) {
    return ptr; // we just waste some RAM in this case
  } else { // if (newsize > oldsize)
    // allocate a new range that's big enough for the new size
    void* newptr = kmalloc(newsize);
    
    // copy anything that was already present
    memcpy(newptr, ptr, oldsize);
    
    // free the old space
    kfree(ptr);
    
    return newptr;
  }
}

void laihost_free(void *ptr, __attribute__((unused)) size_t len)
{
  kfree(ptr);
}

/*
* These functions are very important, but weakly linked,
* so might or might not work as expected
* */

void laihost_log(int lvl, const char *msg)
{
  switch(lvl)
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
