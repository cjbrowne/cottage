#pragma once

#include <stdint.h>
#include <stddef.h>
#include <math/si.h>

typedef struct {
    uint64_t pages;
    uint64_t size;
} malloc_metadata_t;

void *malloc(size_t len);
void* big_malloc(size_t len);

void free(void *ptr);
void big_free(void *ptr);
