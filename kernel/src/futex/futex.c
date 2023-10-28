#include <lock/lock.h>
#include <futex/futex.h>

lock_t futex_lock;

void futex_init()
{
    klog("futex", "Futexes are not yet properly implemented");
}
