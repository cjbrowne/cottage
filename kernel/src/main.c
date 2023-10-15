#include <acpi/acpi.h>
#include <devicetree/dtb.h>
#include <interrupt/idt/idt.h>
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

void* g_framebuffer;

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

    TERM_WRITE_BUF("=== WebOS v0.0.1a (snapshot release) ===\n");

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

    klog("main", "Initializing interrupt handlers");
    // the GDT is provided by limine, and places the kernel in
    // the 64 bit code segment
    // kernel segment is loaded at index 5 (bits 3-15),
    // in the GDT (bit 2),
    // with ring 0 privileges needed for the IDT (bits 0-1)
    idt_init((5 << 3) | 0x00);
    klog("main", "Interrupt handling enabled");


    klog("main", "Initializing PMM");
    pmm_init(memmap_request.response);
    klog("main", "PMM initialized");

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

    klog("main", "Starting network driver");

    net_init();

    klog("main", "Network driver started");

    // we're at the point where we would return control to the bootloader,
    // which makes no sense and may cause damage, so we're going to panic
    // instead.  Make sure there's an infinite loop somewhere above this line!
    panic("kernel returned");
}
