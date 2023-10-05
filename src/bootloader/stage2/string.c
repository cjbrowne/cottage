#include "string.h"
#include <stddef.h>

const char* strchr(const char* str, char chr)
{
    if (str == NULL)
        return NULL;
    while (*str)
    {
        if (*str == chr)
            return str;
        ++str;
    }

    return NULL;
}

char* strcpy(char* dst, const char* src)
{
    // save the start pointer, as we will modify it
    char* dst_start = dst;
    if (dst == NULL)
        return NULL;
    if (src == NULL)
    {
        *dst = '\0';
        return dst;
    }
    while (*src)
    {
        // nb: chris fucked around with this,
        // tutorial code increments both on separate
        // statements.  But there's no need to as the
        // postfix operator already increments _after_ the
        // value is read anyway.
        *dst++ = *src++;
    }

    *dst = '\0';
    // return the pointer to the start of the dst string
    return dst_start;
}

unsigned strlen(const char* str)
{
    unsigned len = 0;
    while(*str++) len++;
    return len;
}
