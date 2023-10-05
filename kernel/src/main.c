#include <acpi/acpi.h>
#include <devicetree/dtb.h>
#include <klog/klog.h>
#include <limine.h>
#include <mem/kmalloc.h>
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

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

static volatile struct limine_paging_mode_request paging_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0,
    .mode = LIMINE_PAGING_MODE_DEFAULT,
    .flags = 0,
};

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {

  // setup kmalloc
  void *kmalloc_start = NULL;

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL ||
      framebuffer_request.response->framebuffer_count < 1) {
    print_serial("ERR: No framebuffer\r\n");
    panic("ERR: No framebuffer\r\n");
  }

  // Fetch the first framebuffer.
  struct limine_framebuffer *framebuffer =
      framebuffer_request.response->framebuffers[0];

  print_serial("Initialising tty\r\n");

  term_init(framebuffer->address, framebuffer->width, framebuffer->height,
            framebuffer->pitch);

  print_serial("Switching to tty print\r\n");

  have_term = true;

  TERM_WRITE_BUF("=== WebOS v0.0.1a (snapshot release) ===\n");
  klog("main", "Loading DTB");
  // Ensure we got a DTB
  if (dtb_request.response == NULL) {
    klog("main", "no DTB present, wrong platform or booting in legacy mode?");
  } else {
    dtb_parse(dtb_request.response->dtb_ptr);
    klog("main", "DTB parsing complete");
  }

  klog("main", "Making sure paging is enabled");
  if(paging_request.response == NULL) {
    panic("Paging disabled.  Paging must be enabled for the kernel to work.");
  }

  klog("main", "Paging mode %x enabled", paging_request.response->mode);

  klog("main", "Loading memory map");
  if (memmap_request.response == NULL) {
    panic("No memory model available, cannot continue boot");
  } else {
    klog("main", "Found %d memory regions",
         memmap_request.response->entry_count);
    for (uint64_t i = memmap_request.response->entry_count; i > 0; i--) {
      struct limine_memmap_entry *entry = memmap_request.response->entries[i];
      if (entry->type == LIMINE_MEMMAP_USABLE) {
        klog("main", "Found %d bytes of usable memory at 0x%x", entry->length,
             entry->base);
        if (entry->length > 0 && kmalloc_start == NULL) {
          kmalloc_start = (void *)entry->base;
          kmalloc_init(kmalloc_start, entry->length);
        }
      }
    }
  }

  have_malloc = true;

  klog("main", "Loading RSDP");
  if (rsdp_request.response == NULL) {
    panic("no RSDP present");
  } else {
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
