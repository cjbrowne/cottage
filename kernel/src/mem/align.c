#include "align.h"

size_t align_up(size_t count, size_t alignment)
{
    return ((count + (alignment - 1)) / alignment) * alignment;
}

size_t align_down(size_t count, size_t alignment)
{
    return (count / alignment) * alignment;
}

size_t div_roundup(size_t a, size_t b)
{
    return (a + (b - 1)) / b;
}
