#include <scheduler/event.h>
#include <klog/klog.h>
#include <panic.h>

_Atomic uint64_t waiting_event_count = 0;

uint64_t event_trigger(event_t* event, bool drop)
{
    klog("event", "event triggered address=%x, drop=%d", event, drop);
    panic("Event triggering is not implemented");
}
