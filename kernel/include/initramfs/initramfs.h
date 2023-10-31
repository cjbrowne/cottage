#pragma once

#include <limine.h>

#define USTAR_FILETYPE_REGULAR_FILE  0x30
#define USTAR_FILETYPE_HARD_LINK     0x31
#define USTAR_FILETYPE_SYM_LINK      0x32
#define USTAR_FILETYPE_CHAR_DEVICE   0x33
#define USTAR_FILETYPE_BLOCK_DEVICE  0x34
#define USTAR_FILETYPE_DIRECTORY     0x35
#define USTAR_FILETYPE_FIFO          0x36
#define USTAR_FILETYPE_GNU_LONG_PATH 0x4C

// omg this thing is huge (ding)
typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char filetype;
    char link_name[100];
    char signature[6];
    char version[2];
    char owner[32];
    char group[32];
    char device_major[8];
    char device_minor[8];
    char prefix[155];
} __attribute__((packed)) ustarheader_t;

void initramfs_init(struct limine_module_response* res);
