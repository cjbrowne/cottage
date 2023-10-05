#include "kmalloc.h"
#include <panic.h>
#include <stdbool.h>
#include <string.h>
#include <klog/klog.h>

typedef struct memory_page {
  void *base;
  bool free;
  size_t count; // used for tracking multi page allocations, usually 0
} memory_page;

static memory_page **page_table;
size_t kernel_pagecount;

#define PAGE_SIZE 512

void kmalloc_init(void *start, size_t len) {

  page_table = start;
  kernel_pagecount = len / PAGE_SIZE;

  size_t page_table_size = sizeof(memory_page) * kernel_pagecount;
  size_t page_table_pages = page_table_size / PAGE_SIZE;
  if (page_table_size % PAGE_SIZE != 0) {
    page_table_pages++;
  }

  klog("mem", "Kernel page table pages: %d", page_table_pages);
  klog("mem", "Allocating %d pages of memory for the kernel", kernel_pagecount);
  klog("mem", "Pages are %d bytes long", PAGE_SIZE);

  // populate the page table
  for (size_t i = 0; i < len / PAGE_SIZE; i++) {
    memory_page page = {
        .free = (i != 0), // page 0 is the page table, so is never free
        .base = start + (i * PAGE_SIZE),
        .count = 0,
    };
    memcpy(start + ((sizeof page) * i), &page, sizeof page);
  }
  
  panic("temp");
  
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
  for (size_t i = 0; i < kernel_pagecount; i++) {
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
  for (size_t i = 0; i < kernel_pagecount; i++) {
    if (page_table[i]->base == ptr) {
      // mark all these pages as free
      for (size_t j = 0; j < page_table[i]->count; j++) {
        page_table[i + j]->free = true;
      }
    }
  }
}