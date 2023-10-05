#pragma once

#include <stdint.h>

typedef struct {
    char signature[8];
    uint8_t checksum;
    char OEM_ID[6];
    uint8_t revision;
    uint32_t RSDT_address;

    uint32_t length;
    uint64_t XSDT_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__ ((packed)) XSDP_t;

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEM_ID[6];
    char OEM_table_ID[8];
    uint32_t OEM_revision;
    uint32_t creator_ID;
    uint32_t creator_revision;
} ACPISDTHeader;

typedef struct {
    ACPISDTHeader header;
    ACPISDTHeader* other_sdts[];
} RSDT_t;

void acpi_init(XSDP_t* xsdp_table);