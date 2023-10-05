#include "idt.h"
#include <stdint.h>
#include <util/binary.h>

typedef struct {
    uint16_t base_low;              // the offset (low 16 bits)
    uint16_t segment_selector;      // the segment
    uint8_t reserved;               // this byte is always 0
    uint8_t flags;                  // flags (gate type, 0, DPL, P)
    uint16_t base_high;             // the offset (high 16 bits)
} __attribute__((packed)) IDT_entry;


typedef struct {
    uint16_t limit;
    IDT_entry* ptr;
} __attribute__((packed)) IDT_descriptor;


IDT_entry g_IDT[256];

IDT_descriptor g_IDTDescriptor = { sizeof(g_IDT) - 1, g_IDT };

void __attribute__((cdecl)) i686_IDT_Load(IDT_descriptor* idt_desc);

void i686_IDT_SetGate(int interrupt, void* base, uint16_t segment_descriptor, uint8_t flags)
{
    g_IDT[interrupt].base_low = ((uint32_t) base) & 0xFFFF;
    g_IDT[interrupt].segment_selector = segment_descriptor;
    g_IDT[interrupt].reserved = 0;
    g_IDT[interrupt].flags = flags;
    g_IDT[interrupt].base_high = ((uint32_t) base >> 16) & 0xFFFF;
}

void i686_IDT_EnableGate(int interrupt)
{
    FLAG_SET(g_IDT[interrupt].flags, IDT_FLAG_PRESENT);
}

void i686_IDT_DisableGate(int interrupt)
{
    FLAG_UNSET(g_IDT[interrupt].flags, IDT_FLAG_PRESENT);
}

void i686_IDT_init()
{
    i686_IDT_Load(&g_IDTDescriptor);

}