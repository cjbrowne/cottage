#include <string.h>
#include <term/term.h>
#include <klog/klog.h>
#include "acpi.h"

void acpi_init(XSDP_t* xsdp_table)
{
    RSDT_t* rsdt;
    if (xsdp_table->revision <= 2)
    {
        klog("acpi", "Assuming ACPI 1.0");
        term_printf("ACPI Revision: %d\n", xsdp_table->revision);
        rsdt = (RSDT_t *) (xsdp_table->RSDT_address);
        if (memcmp(xsdp_table->signature, "RSD PTR ", 8) != 0)
        {
            klog("acpi", "ACPI signature match error");
            term_printf("Signature: %x\n", xsdp_table->signature);
        }
    }
    else
    {

    }

}