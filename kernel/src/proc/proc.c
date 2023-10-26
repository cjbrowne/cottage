#include <proc/proc.h>

thread_t* get_current_thread()
{
    thread_t* ret = NULL;

    asm volatile (
        "movq %%gs:[8], %0"
        : "=r" (ret)
    );

    return ret;
}


