#pragma once

#include <stddef.h>
#include <macro.h>
#include <acpispec/tables.h>

typedef struct {
    acpi_header_t header;
    uint32_t local_controller_addr;
    uint32_t flags;
    uint8_t entries_begin;
} __attribute__((packed)) madt_t;

typedef struct {
    uint8_t id;
    uint8_t len;
} __attribute__((packed)) madt_header_t;

typedef struct {
    madt_header_t header;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) madt_local_apic_t;

typedef struct {
    madt_header_t header;
    uint8_t apic_id;
    RESERVE_BYTES(1);
    uint32_t address;
    uint32_t gsib;
} __attribute__((packed)) madt_io_apic_t;

typedef struct {
    madt_header_t header;
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t gsi;
    uint16_t flags;
} __attribute__((packed)) madt_iso_t;

typedef struct {
    madt_header_t header;
    uint8_t processor;
    uint16_t flags;
    uint8_t lint;
} __attribute__((packed)) madt_nmi_t;

extern madt_t* madt;
extern madt_local_apic_t** madt_local_apics;
extern madt_io_apic_t** madt_io_apics;
extern madt_iso_t** madt_isos;
extern madt_nmi_t** madt_nmis;
extern size_t madt_local_apic_count;
extern size_t madt_io_apic_count;
extern size_t madt_iso_count;
extern size_t madt_nmi_count;


void madt_init();




