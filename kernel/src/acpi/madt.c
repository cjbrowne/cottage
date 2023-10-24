#include <acpi/madt.h>

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <mem/malloc.h>
#include <acpi/acpi.h>
#include <klog/klog.h>

madt_t* madt;
madt_local_apic_t* madt_local_apics;
madt_io_apic_t* madt_io_apics;
madt_iso_t* madt_isos;
madt_nmi_t* madt_nmis;
size_t madt_local_apic_count = 0;
size_t madt_io_apic_count = 0;
size_t madt_iso_count = 0;
size_t madt_nmi_count = 0;

void madt_init()
{
    // the MADT is inexplicably called APIC.  This cost me 3 hours to discover.
    madt = (madt_t*) acpi_find_table("APIC", 0);
    if(madt == NULL) 
    {
        panic("System does not have an MADT!");
    }

    klog("acpi/madt", "MADT is %d bytes long", madt->header.length);

    uint64_t cur = 0;
    do
    {
        if(cur + (sizeof(madt_t) - 1) >= madt->header.length) break;
        madt_header_t* header = (madt_header_t*)(((uint64_t)&madt->entries_begin) + cur);
        klog("acpi/madt", "Found header id=%d len=%d", header->id, header->len);

        switch(header->id)
        {
            case 0:
                klog("acpi/madt", "Found local APIC %x", madt_local_apic_count);
                madt_local_apic_count++;
                madt_local_apics = realloc(madt_local_apics, sizeof(madt_local_apic_t) * madt_local_apic_count);
                memcpy(&madt_local_apics[madt_local_apic_count-1], header, sizeof(madt_local_apic_t));
                break;
            case 1:
                klog("acpi/madt", "Found IO APIC %x", madt_io_apic_count);
                madt_io_apic_count++;
                madt_io_apics = realloc(madt_io_apics, sizeof(madt_io_apic_t) * madt_io_apic_count);
                memcpy(&madt_io_apics[madt_io_apic_count-1], header, sizeof(madt_io_apic_t));
                break;
            case 2:
                klog("acpi/madt", "Found ISO %x", madt_iso_count);
                madt_iso_count++;
                madt_isos = realloc(madt_isos, sizeof(madt_iso_t) * madt_iso_count);
                memcpy(&madt_isos[madt_iso_count-1], header, sizeof(madt_iso_t));
                break;
            case 3:
                klog("acpi/madt", "Found NMI %x", madt_nmi_count);
                madt_nmi_count++;
                madt_nmis = realloc(madt_nmis, sizeof(madt_nmi_t) * madt_nmi_count);
                memcpy(&madt_nmis[madt_nmi_count-1], header, sizeof(madt_nmi_t));
                break;
            default:
                klog("acpi/madt", "WARNING! Found unknown MADT header %d", header->id);
                break;
        }

        cur += (uint64_t) header->len;
    } while(true);
}
