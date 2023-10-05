#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <term/term.h>
#include <devicetree/dtb.h>
#include <klog/klog.h>
#include <net/network.h>
#include <acpi/acpi.h>
#include <serial/serial.h>


// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_framebuffer_request framebuffer_request = {
        .id = LIMINE_FRAMEBUFFER_REQUEST,
        .revision = 0
};

static volatile struct limine_dtb_request dtb_request = {
        .id = LIMINE_DTB_REQUEST,
        .revision = 0,
};

static volatile struct limine_rsdp_request rsdp_request = {
        .id = LIMINE_RSDP_REQUEST,
        .revision = 0,
};

// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
        || framebuffer_request.response->framebuffer_count < 1) {
        print_serial("ERR: No framebuffer\r\n");
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    print_serial("Initialising tty\r\n");


    term_init(
            framebuffer->address,
            framebuffer->width,
            framebuffer->height,
            framebuffer->pitch
    );

    print_serial("Switching to tty print\r\n");

    TERM_WRITE_BUF("=== WebOS v0.0.1a (snapshot release) ===\n");
    klog("main", "Loading DTB");
    // Ensure we got a DTB
    if (dtb_request.response == NULL) {
        klog("main", "no DTB present, wrong platform or booting in legacy mode?");
    } else {
        dtb_parse(dtb_request.response->dtb_ptr);
        klog("main", "DTB parsing complete");
    }

    klog("main", "Loading RSDP");
    if (rsdp_request.response == NULL) {
        klog("main", "no RSDP present");
        hcf(); // panic
    } else {
        acpi_init(rsdp_request.response->address);
    }

    klog("main", "Starting network driver");

    net_init();

    klog("main", "Network driver started");

    // We're done, just hang...
    hcf();
}
