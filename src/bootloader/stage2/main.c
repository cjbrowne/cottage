#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "memdetect.h"
#include <boot/bootparams.h>

uint8_t* KernelLoadBuffer   = (uint8_t*) MEMORY_LOAD_KERNEL;
uint8_t* Kernel             = (uint8_t*) MEMORY_KERNEL_ADDR;

boot_params g_BootParams;

// typedef for the kernel itself
typedef void (*KernelStart)(boot_params* bp);

_Noreturn void __attribute__((cdecl)) start(uint16_t boot_drive)
{
    clrscr();
    printf("=== Stage 2 Loaded ===\r\n");
    printf("Initialising disk...");
    DISK disk;
    if(!DISK_init(&disk, boot_drive))
    {
            printf("Disk init error\r\n");
            goto end;
    }
    printf("Done\r\n");

    printf("Initialising FAT filesystem...");
    if(!FAT_init(&disk))
    {
        printf("FAT init error\r\n");
        goto end;
    }
    printf("Done\r\n");

    printf("Preparing boot params...\r\n");
        printf("\t\tSetting boot device...");
        g_BootParams.boot_device = boot_drive;
        printf("Done\r\n");
        printf("\t\tDetecting memory...\r\n");
        memory_detect(&g_BootParams.memory);
        printf("\t\tDone\r\n");
    printf("Done\r\n");

    printf("Loading Kernel...");
    FAT_File* fd = FAT_Open(&disk, "/kernel.bin");
    uint32_t read;
    uint8_t* kernel_buffer = Kernel;
    while ((read = FAT_read(&disk, fd, MEMORY_LOAD_SIZE, KernelLoadBuffer)))
    {

        memcpy(kernel_buffer, KernelLoadBuffer, read);
        kernel_buffer += read;

    }
    FAT_close(fd);
    printf("Done\r\n");

    printf("Entering kernel.");
    KernelStart kernelStart = (KernelStart)Kernel;
    printf(".");
    kernelStart(&g_BootParams);
end:
    printf("Something went pete tong.  Please restart your computer.\r\n");
    for (;;);
}
