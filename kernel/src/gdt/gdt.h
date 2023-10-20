#pragma once

#include <stdint.h>

typedef struct {
    uint16_t size;
    void* address;
} __attribute__((packed)) gdt_pointer_t;

typedef struct {
    uint16_t limit;
    uint16_t base_low_w;
    uint8_t base_mid_b;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high_b;
} __attribute__((packed)) gdt_entry_t;

#define KERNEL_CODE_SEGMENT 0x28
#define KERNEL_DATA_SEGMENT 0x30
#define USER_CODE_SEGMENT 0x43
#define USER_DATA_SEGMENT 0x3b
#define TSS_SEGMENT 0x48

#define GDT_ACCESS_PRESENT (1 << 7)
#define GDT_ACCESS_RING0 (0 << 5)
#define GDT_ACCESS_RING1 (1 << 5)
#define GDT_ACCESS_RING2 (2 << 5)
#define GDT_ACCESS_RING3 (3 << 5)
#define GDT_ACCESS_DESCRIPTOR_TYPE_TSS (0 << 4)
#define GDT_ACCESS_DESCRIPTOR_TYPE_CODE_OR_DATA (1 << 4)
#define GDT_ACCESS_EXECUTABLE (1 << 3)
#define GDT_ACCESS_DIRECTION_DOWN (1 << 2)
#define GDT_ACCESS_RW_ENABLE (1 << 1)
#define GDT_ACCESS_ACCESSED (1 << 0)

#define GDT_GRANULARITY_LIMIT_PAGES (1 << 7)
#define GDT_GRANULARITY_32BIT (1 << 6)
#define GDT_GRANULARITY_64BIT_CODE (1 << 5)

#define GDT_GRANULARITY_LIMIT_HI 0xF
#define GDT_GRANULARITY_LIMIT_LO 0

extern gdt_pointer_t gdt_pointer;
extern gdt_entry_t gdt_entries[11];

void gdt_init();
void gdt_reload();
