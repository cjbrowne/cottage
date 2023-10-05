#include "gdt.h"
#include <stdint.h>

typedef struct {
    uint16_t limit_low;             // limit (bits 0-15)
    uint16_t base_low;              // base  (bits 0-15)
    uint8_t base_middle;            // base (bits 16-23)
    uint8_t access;                 // access flags
    uint8_t flags_limit_high;       // limit (bits 16-19) | flags
    uint8_t base_high;              // base (bits 24-31)
} __attribute__((packed)) GDT_entry;

typedef struct {
    uint16_t limit;                 // sizeof(gdt) - 1
    GDT_entry *ptr;                 // address of GDT
} __attribute__((packed)) GDT_descriptor;

typedef enum {
    GDT_ACCESS_CODE_READABLE = 0x02,
    GDT_ACCESS_DATA_WRITEABLE = 0x02,

    GDT_ACCESS_CODE_CONFORMING = 0x04, // can code be executed in lower privilege ring?
    GDT_ACCESS_DATA_DIRECTION_NORMAL = 0x00, // segment goes upwards
    GDT_ACCESS_DATA_DIRECTION_DOWN = 0x04, // segment goes downwards

    GDT_ACCESS_DATA_SEGMENT = 0x10,     // descriptor bit set, executable bit unset
    GDT_ACCESS_CODE_SEGMENT = 0x18,     // descriptor bit set, executable bit set

    GDT_ACCESS_DESCRIPTOR_TSS = 0x00,     // descriptor bit unset, executable bit unset == TSS (Task State Segment)

    GDT_ACCESS_RING0 = 0x00,     // kernel
    GDT_ACCESS_RING1 = 0x20,     // driver
    GDT_ACCESS_RING2 = 0x40,     // system
    GDT_ACCESS_RING3 = 0x60,     // userland

    GDT_ACCESS_PRESENT = 0x80,     // segment enabled (used to dynamically enable/disable segments)
} GDT_ACCESS;


typedef enum {
    GDT_FLAG_64BIT = 0x20,
    GDT_FLAG_32BIT = 0x40,
    GDT_FLAG_16BIT = 0x00,

    GDT_FLAG_GRANULARITY_1B = 0x00,
    GDT_FLAG_GRANULARITY_4K = 0x80,
} GDT_FLAGS;

// helper macros
#define GDT_LIMIT_LOW(limit)                    (limit & 0xFFFF)
#define GDT_BASE_LOW(base)                      (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)                   ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)        (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                     ((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) { \
                                        GDT_LIMIT_LOW(limit),               \
                                        GDT_BASE_LOW(base),                 \
                                        GDT_BASE_MIDDLE(base),              \
                                        access,                             \
                                        GDT_FLAGS_LIMIT_HI(limit, flags),   \
                                        GDT_BASE_HIGH(base)                 \
                                        }

// set up _the_ GDT
GDT_entry g_GDT[] = {
        // NULL descriptor
        GDT_ENTRY(0, 0, 0, 0),

        // Kernel 32-bit code segment
        GDT_ENTRY(0,
                  0xFFFF,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
                  GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

        // Kernel 32-bit data segment
        GDT_ENTRY(0,
                  0xFFFF,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
                  GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),
};

GDT_descriptor g_GDTDescriptor = { sizeof(g_GDT) - 1, g_GDT };

void __attribute__((cdecl)) i686_GDT_Load(GDT_descriptor* descriptor, uint16_t code_segment, uint16_t data_segment);

void i686_GDT_Initialize()
{
    i686_GDT_Load(&g_GDTDescriptor, i686_GDT_CODE_SEGMENT, i686_GDT_DATA_SEGMENT);
}