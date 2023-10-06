#include "acpi.h"
#include <klog/klog.h>
#include <lai/helpers/sci.h>
#include <lai/host.h>
#include <panic.h>
#include <stdint.h>
#include <string.h>
#include <term/term.h>

#define LAI_ACPI_MODE_LEGACY_8259 0
#define LAI_ACPI_MODE_APIC 1
#define LAI_ACPI_MODE_SAPIC 2

void acpi_init(__attribute__((unused)) XSDP_t *xsdp_table) {
  klog("acpi", "Enabling ACPI mode in LAI");
  lai_enable_acpi(LAI_ACPI_MODE_SAPIC);
  klog("acpi", "ACPI mode enabled");
}