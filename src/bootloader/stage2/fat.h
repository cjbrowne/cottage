#pragma once
#include "stdint.h"
#include "disk.h"

#pragma pack(push, 1)

typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t created_time_tenths;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t first_cluster_low;
    uint32_t size;
} FAT_direntry;

#pragma pack(pop)

typedef struct
{
    int handle;
    bool is_dir;
    uint32_t pos;
    uint32_t size;
} FAT_File;

enum FAT_Attributes
{
    FAT_ATTR_READ_ONLY              = 0x01,
    FAT_ATTR_HIDDEN                 = 0x02,
    FAT_ATTR_SYSTEM                 = 0x04,
    FAT_ATTR_VOLUME_ID              = 0x08,
    FAT_ATTR_DIRECTORY              = 0x10,
    FAT_ATTR_ARCHIVE                = 0x20,
    FAT_ATTR_LFN                    = FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID
};

bool FAT_init(DISK* disk);
FAT_File* FAT_open_entry(DISK* disk, FAT_direntry* entry);
FAT_File* FAT_Open(DISK* disk, const char* path);
uint32_t FAT_read(DISK *disk, FAT_File* file, uint32_t byte_count, void* out_buf);
bool FAT_readdir(DISK *disk, FAT_File* file, FAT_direntry* out);
void FAT_close(FAT_File* file);
