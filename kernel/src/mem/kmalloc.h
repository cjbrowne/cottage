#pragma once

#include <stddef.h>
#include <math/si.h>

void *kmalloc(size_t len);

void kfree(void *ptr);
