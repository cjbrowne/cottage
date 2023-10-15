#include "align.h"

size_t align_up(size_t count, size_t alignment)
{
    return ((count + (alignment - 1)) / alignment) * alignment;
}
