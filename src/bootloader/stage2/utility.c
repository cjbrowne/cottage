#include "utility.h"

uint32_t align(uint32_t number, uint32_t align_to)
{
    if(align_to == 0)
    return number;
    uint32_t rem = number % align_to;

    return (rem > 0) ? (number + align_to - rem) : number;
}
