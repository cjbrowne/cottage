#pragma once

#include <stddef.h>
#include <stdint.h>
#include <acpispec/tables.h>

void acpi_init(acpi_xsdp_t *xsdp_table);
void *acpi_find_table(char *sig, size_t idx);
