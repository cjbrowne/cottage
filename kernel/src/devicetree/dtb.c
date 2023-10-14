#include "dtb.h"

// all of this shit is unused tbh - candidate for removal maybe?
// we do want to support ARM eventually, but maybe we can just do that when
// we do that and not try to build stubbed arm support in this early on?
void dtb_parse(__attribute__((unused)) void *dtb)
{
    // not implemented (not used by x86 platform)
}
