#include <acpi/acpi.h>
#include <scheduler/scheduler.h>
#include <devicetree/dtb.h>
#include <interrupt/idt.h>
#include <interrupt/isr.h>
#include <klog/klog.h>
#include <limine.h>
#include <mem/vmm.h>
#include <mem/pmm.h>
#include <net/network.h>
#include <panic.h>
#include <serial/serial.h>
#include <stddef.h>
#include <stdint.h>
#include <term/term.h>
#include <pci/pci.h>
#include <cpu/smp.h>
#include <gdt/gdt.h>
#include <time/pit.h>
#include <time/timer.h>

// hardware-specific stuff 
// todo: move this shit behind HAL and/or into modules
#include <drivers/e1000/e1000.h>

// global var used to check in certain low level functions (e.g panic)
// whether we have a terminal to write to or not
bool have_term = false;

// global var used to check in certain low level functions (e.g klog)
// whether we have a kmalloc set up yet or not.
// when kmalloc is not available, we have to fall back on static allocation,
// or truncate our output.
bool have_malloc = false;

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

static volatile struct limine_dtb_request dtb_request = {
    .id = LIMINE_DTB_REQUEST,
    .revision = 0,
};

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

static volatile struct limine_paging_mode_request paging_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0,
    .mode = LIMINE_PAGING_MODE_DEFAULT,
    .flags = 0,
};

static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};

static volatile struct limine_boot_time_request boottime_request = {
    .id = LIMINE_BOOT_TIME_REQUEST,
    .revision = 0,
};

void* g_framebuffer;

void kmain_thread();

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void)
{

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1)
    {
        print_serial("ERR: No framebuffer\r\n");
        panic("ERR: No framebuffer\r\n");
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer =
        framebuffer_request.response->framebuffers[0];

    g_framebuffer = framebuffer->address;

    print_serial("Initialising tty\r\n");

    term_init(framebuffer->address, framebuffer->width, framebuffer->height,
              framebuffer->pitch);

    print_serial("Switching to tty print\r\n");

    have_term = true;

    klog("main", "framebuffer address=%x width=%d height=%d pitch=%d pixel_width=%d", 
            framebuffer->address, 
            framebuffer->width, 
            framebuffer->height, 
            framebuffer->pitch,
            (framebuffer->bpp / 8)
            );

    TERM_WRITE_BUF("=== Hopper v0.0.1a (snapshot release) ===\n");

    klog("main", "Loading DTB");
    // Ensure we got a DTB
    if (dtb_request.response == NULL)
    {
        klog("main", "no DTB present, wrong platform or booting in legacy mode?");
    }
    else
    {
        dtb_parse(dtb_request.response->dtb_ptr);
        klog("main", "DTB parsing complete");
    }

    klog("main", "Making sure paging is enabled");
    if (paging_request.response == NULL)
    {
        panic("Paging disabled.  Paging must be enabled for the kernel to work.");
    }

    klog("main", "Paging mode %x enabled", paging_request.response->mode);

    if(memmap_request.response == NULL)
    {
        panic("Memmap missing.  Cannot determine where to map physical memory to virtual memory");
    }

    klog("main", "Initializing PMM");
    pmm_init(memmap_request.response);
    klog("main", "PMM initialized");

    klog("main", "Initializing GDT");
    gdt_init();
    klog("main", "GDT Initialized");

    klog("main", "Initializing interrupt handlers");
    idt_init();
    klog("main", "Interrupt table loaded");
    isr_init();
    klog("main", "Interrupts enabled");



    klog("main", "Initializing VMM");
    if (kernel_address_request.response == NULL)
    {
        klog("main", "No kernel address");
        panic("Can't init VMM");
    }
    vmm_init(
        kernel_address_request.response->physical_base, 
        kernel_address_request.response->virtual_base,
        memmap_request.response
        );
    klog("main", "VMM initialized");    

    have_malloc = true;
    

    klog("main", "Loading RSDP");
    if (rsdp_request.response == NULL)
    {
        panic("no RSDP present");
    }
    else
    {
        klog("main", "Found RSDP at %x", rsdp_request.response->address);
        acpi_init(rsdp_request.response->address);
    }

    klog("main", "Enumerating PCI devices");
    pci_init();
    klog("main", "PCI devices enumerated");

    klog("main", "Initializing SMP");
    klog("main", "SMP response was: %x", smp_request.response);
    smp_init(smp_request.response);
    klog("main", "SMP initialized");

    klog("main", "Initializing high resolution timer");
    if(boottime_request.response == NULL)
    {
        klog("main", "WARNING! Unknown boot time, using 0 as epoch");
        timer_init(0);
    }
    else
    {
        timer_init(boottime_request.response->boot_time);
    }
    klog("main", "High-resolution timer initialized");

    klog("main", "Initializing scheduler");
    scheduler_init();
    klog("main", "Scheduler initialized");

    klog("main", "Kernel main thread starts at %x", kmain_thread);
    new_kernel_thread((void*)kmain_thread, NULL, true);

    klog("main", "Startup complete");
    scheduler_await();

    // we're at the point where we would return control to the bootloader,
    // which makes no sense and may cause damage, so we're going to panic
    // instead.  Make sure there's an infinite loop somewhere above this line!
    panic("kernel returned");
}

void kmain_thread()
{
    panic("Implement kmain_thread");
}
