#include <stdint.h>
#include <stddef.h>
#include <klog/klog.h>
#include <string.h>

// this doesn't always exist in string.h, as it is a GNU extension
void* mempcpy(void *dest, const void *src, size_t n);

int strcmp(const char* s1, const char* s2)
{
    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);

    size_t min_len = s1_len < s2_len ? s1_len : s2_len;

    int cmp_result = memcmp(s1, s2, min_len);

    if(cmp_result == 0)
    {
        // s1 is a prefix of s2
        if(s1_len < s2_len) return -1;
        // s2 is a prefix of s1
        else if (s1_len > s2_len) return 1;
        // the lengths are equal, the strings match
        else return 0;
    }

    // s1 and s2 do not match
    return cmp_result;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    mempcpy(dest, src, n);
    return dest;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest)
    {
        for (size_t i = 0; i < n; i++)
        {
            pdest[i] = psrc[i];
        }
    }
    else if (src < dest)
    {
        for (size_t i = n; i > 0; i--)
        {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) 
{
    return memcmp((void*)s1, (void*)s2, n);
}

// unsafe strlen function, counts until it hits a NULL byte with no limit
size_t strlen(const char *str)
{
    const char *s = str;
    while(*s) s++;
    return s - str;
}

void* mempcpy(void *dest, const void *src, size_t n)
{
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++)
    {
        *pdest = *psrc;
        pdest++;
        psrc++;
    }

    return pdest;
}

char* stpcpy(char* restrict dest, const char* restrict src)
{
    char *p;
    p = mempcpy(dest, src, strlen(src));
    *p = '\0';

    return p;
}

char* strcpy(char* restrict dest, const char* restrict src)
{
    stpcpy(dest, src);
    return dest;
}
