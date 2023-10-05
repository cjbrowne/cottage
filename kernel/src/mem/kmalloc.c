#include "kmalloc.h"
#include <klog/klog.h>
#include <math/si.h>
#include <panic.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct memory_page {
  void *base;
  bool free;
  size_t count; // used for tracking multi page allocations, usually 0
} memory_page;

// heap size in bytes
#define KMALLOC_HEAP_SIZE MiB(2)

// use any po2 for page size - non-po2s could cause alignment issues
#define PAGE_SIZE 512

#define PAGE_COUNT (KMALLOC_HEAP_SIZE / PAGE_SIZE)

__attribute__((section(".heap"))) static uint8_t kmalloc_mem[KMALLOC_HEAP_SIZE];

static memory_page *page_table[KMALLOC_HEAP_SIZE / PAGE_SIZE];
static size_t free_pages = 0;

size_t get_free_pages() { return free_pages; }

size_t kmalloc_init() {

  size_t page_table_size = sizeof(memory_page) * PAGE_COUNT;
  size_t page_table_pages = page_table_size / PAGE_SIZE;
  if (page_table_size % PAGE_SIZE != 0) {
    page_table_pages++;
  }

  klog("mem", "kmalloc info:");
  klog("mem", "Kernel page table pages: %d", page_table_pages);
  klog("mem", "Allocating %d pages of memory for the kernel", PAGE_COUNT);
  klog("mem", "Pages are %d bytes long", PAGE_SIZE);

  // populate the page table
  for (size_t i = 0; i < PAGE_COUNT; i++) {
    memory_page page = {
        // mark page table pages as used
        .free = (i > page_table_pages),
        .base = kmalloc_mem + (i * PAGE_SIZE),
        .count = 0,
    };
    memcpy(kmalloc_mem + ((sizeof page) * i), &page, sizeof page);
  }

  free_pages = PAGE_COUNT - page_table_pages;

  return KMALLOC_HEAP_SIZE;
}

void *kmalloc(size_t len) {
  size_t pages_needed = len / PAGE_SIZE;
  // if we have stray bytes after the last page,
  // we need an extra page for those
  if (len % PAGE_SIZE != 0) {
    pages_needed++;
  }
  // todo: highly naive and very basic memory allocator
  // can only allocate an entire page at a time
  for (size_t i = 0; i < PAGE_COUNT; i++) {
    size_t pages_allocated = 0;
    for (pages_allocated = 0; pages_allocated < pages_needed;
         pages_allocated++) {
      if (!page_table[i + pages_allocated]->free)
        break;
    }
    if (pages_allocated == pages_needed) {
      // set the pages as allocated
      for (pages_allocated = 0; pages_allocated < pages_needed;
           pages_allocated++) {
        page_table[i + pages_allocated]->free = false;
        free_pages--;
      }
      page_table[i]->count = pages_needed;
      return page_table[i]->base;
    }
  }
  panic("Kernel OOM");
  // this return technically can't be reached because of the above panic
  return NULL;
}

void kfree(void *ptr) {
  for (size_t i = 0; i < PAGE_COUNT; i++) {
    if (page_table[i]->base == ptr) {
      // mark all these pages as free
      for (size_t j = 0; j < page_table[i]->count; j++) {
        page_table[i + j]->free = true;
        free_pages++;
      }
    }
  }
}