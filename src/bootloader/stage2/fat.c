#include "fat.h"
#include "disk.h"
#include "stdio.h"
#include "memdefs.h"
#include "string.h"
#include "memory.h"
#include "ctype.h"
#include "minmax.h"
#include <stdbool.h>

#define SECTOR_SIZE             512
#define MAX_PATH_SIZE           256
#define MAX_FILE_HANDLES        10
#define ROOT_DIRECTORY_HANDLE   -1

typedef struct {
    uint8_t boot_jump_instruction[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t dir_entry_count;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_counts;
    uint8_t drive_number;
    uint8_t _reserved; // never used
    uint8_t signature; // always 0x29
    uint32_t volume_id; // serial number
    uint8_t volume_label[11];
    uint8_t system_id[8];
} __attribute__((packed)) FAT_boot_sector;

typedef struct {
    // "public" name is weird, refactor later
    FAT_File    public;
    bool        opened;
    uint32_t    first_cluster;
    uint32_t    current_cluster;
    uint32_t    current_sector_in_cluster;
    uint8_t     buf[SECTOR_SIZE];
} FAT_FileData;

// don't like this "hold-all" style of struct,
// refactor later
typedef struct {
    union {
        FAT_boot_sector boot_sector;
        uint8_t boot_sector_bytes[SECTOR_SIZE];
    } BS;

    FAT_FileData root_directory;
    FAT_FileData open_files[MAX_FILE_HANDLES];
} FAT_data;

// don't like this variable naming, refactor later
// it does need to be global though
static FAT_data *g_Data;
static uint8_t * g_Fat = NULL;
static uint32_t g_DataSectionLba;

bool FAT_read_boot_sector(DISK *disk) {
    return DISK_read_sectors(disk, 0, 1, (uint8_t*)&g_Data->BS.boot_sector_bytes);
}

bool FAT_read_fat(DISK *disk) {
    return DISK_read_sectors(disk, g_Data->BS.boot_sector.reserved_sectors, g_Data->BS.boot_sector.sectors_per_fat, g_Fat);
}

bool FAT_init(DISK *disk) {
    // store all the pointer data at the start of MEMORY_FAT_ADDR
    g_Data = (FAT_data*)MEMORY_FAT_ADDR;

    if (!FAT_read_boot_sector(disk)) {
        printf("FAT: read boot sector failed\r\n");
        return false;
    }

    g_Fat = (uint8_t *)g_Data + sizeof(FAT_data);
    uint32_t fat_size = g_Data->BS.boot_sector.sectors_per_fat * g_Data->BS.boot_sector.bytes_per_sector;
    if (sizeof(FAT_data) + fat_size >= MEMORY_FAT_SIZE) {
        printf("FAT: Not enough memory\r\n");
        return false;
    }

    if (!FAT_read_fat(disk)) {
        printf("FAT: Could not read FAT\r\n");
        return false;
    }

    // root dir
    uint32_t root_dir_lba = g_Data->BS.boot_sector.reserved_sectors + g_Data->BS.boot_sector.sectors_per_fat * g_Data->BS.boot_sector.fat_count;
    uint32_t root_dir_size = sizeof(FAT_direntry) * g_Data->BS.boot_sector.dir_entry_count;

    // open root directory file
    g_Data->root_directory.public.handle                = ROOT_DIRECTORY_HANDLE;
    g_Data->root_directory.public.is_dir                = true;
    g_Data->root_directory.public.pos                   = 0;
    g_Data->root_directory.public.size                  = root_dir_size;
    g_Data->root_directory.opened                       = true;
    g_Data->root_directory.first_cluster                = root_dir_lba;
    g_Data->root_directory.current_cluster              = root_dir_lba;
    g_Data->root_directory.current_sector_in_cluster    = 0;

    // read the root directory
    if(!DISK_read_sectors(disk, root_dir_lba, 1, g_Data->root_directory.buf))
    {
        printf("FAT: could not read root directory\r\n");
        return false;
    }

    // calculate data section
    uint32_t root_dir_sectors = (root_dir_size + g_Data->BS.boot_sector.bytes_per_sector - 1) / g_Data->BS.boot_sector.bytes_per_sector;
    g_DataSectionLba = root_dir_lba + root_dir_sectors;

    // reset opened files
    for (int i = 0; i < MAX_FILE_HANDLES; i++)
        g_Data->open_files[i].opened = false;

    return true;
}

uint32_t FAT_ClusterToLba(uint32_t cluster)
{
    return g_DataSectionLba + (cluster - 2) * g_Data->BS.boot_sector.sectors_per_cluster;
}

FAT_File* FAT_open_entry(DISK* disk, FAT_direntry* entry)
{
    // find empty handle
    int handle = -1;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++)
    {
        if(!g_Data->open_files[i].opened)
            handle = i;
    }

    if (handle < 0)
    {
        printf("FAT open file error: No more file handles\r\n");
        return NULL;
    }

    // setup vars
    FAT_FileData* fd = &g_Data->open_files[handle];
    fd->public.handle = handle;
    fd->public.is_dir = (entry->attributes & FAT_ATTR_DIRECTORY) != 0;
    fd->public.pos = 0;
    fd->public.size = entry->size;
    fd->first_cluster = entry->first_cluster_low + ((uint32_t)entry->first_cluster_high << 16);
    fd->current_cluster = fd->first_cluster;
    fd->current_sector_in_cluster = 0;

    if(!DISK_read_sectors(disk, FAT_ClusterToLba(fd->current_cluster), 1, fd->buf))
    {
        printf("FAT: open entry failed - read error cluster=%u lba=%u\n", fd->current_cluster, FAT_ClusterToLba(fd->current_cluster));
        for (int i = 0; i < 11; i++)
            printf("%c", entry->name[i]);
        printf("\n");
        return NULL;
    }

    fd->opened = true;
    return &fd->public;
}

uint32_t FAT_next_cluster(uint32_t current_cluster)
{
    uint32_t fat_index = current_cluster * 3 / 2;
    if (current_cluster % 2 == 0) {
        return (*(uint16_t*) (g_Fat + fat_index)) & 0x0FFF;
    } else {
        return (*(uint16_t*) (g_Fat + fat_index)) >> 4;
    }
}

uint32_t FAT_read(DISK *disk, FAT_File* file, uint32_t byte_count, void* out_buf)
{

    // get file data
    FAT_FileData* fd =
        (file->handle == ROOT_DIRECTORY_HANDLE) ?
        &g_Data->root_directory :
        &g_Data->open_files[file->handle];

    uint8_t* u8_data_out = (uint8_t*) out_buf;

    // don't read past the end of the file
    if(!fd->public.is_dir || (fd->public.is_dir && fd->public.size != 0))
        byte_count = min(byte_count, fd->public.size - fd->public.pos);

    while(byte_count > 0) {
        uint32_t left_in_buffer = SECTOR_SIZE - (fd->public.pos % SECTOR_SIZE);
        uint32_t take = min(byte_count, left_in_buffer);

        memcpy(u8_data_out, fd->buf + fd->public.pos % SECTOR_SIZE, take);
        u8_data_out += take;
        fd->public.pos += take;
        byte_count -= take;

        // printf("leftInBuffer=%lu take=%lu\r\n", left_in_buffer, take);

        // do we need to read any more?
        if (left_in_buffer == take)
        {
            // special handling for root directory
            if (fd->public.handle == ROOT_DIRECTORY_HANDLE)
            {
                ++fd->current_cluster;
                if(!DISK_read_sectors(disk, fd->current_cluster, 1, fd->buf))
                {
                    printf("FAT: could not read sector from file\r\n");
                    break;
                }
            }
            else
            {
                if (++fd->current_sector_in_cluster >= g_Data->BS.boot_sector.sectors_per_cluster)
                {
                    fd->current_sector_in_cluster = 0;
                    fd->current_cluster = FAT_next_cluster(fd->current_cluster);
                }

                if (fd->current_cluster >= 0xFF8)
                {
                    // mark end of file at this point (cannot read any more from disk)
                    fd->public.size = fd->public.pos;
                    break;
                }

//                printf("Read sector cluster=%d sector=%d spc=%d\r\n", fd->current_cluster, fd->current_sector_in_cluster);
                if(!DISK_read_sectors(disk, FAT_ClusterToLba(fd->current_cluster) + fd->current_sector_in_cluster, 1, fd->buf))
                {
                    printf("FAT: could not read sector from file\r\n");
                    break;
                }
            }
        }
    }

    // number of bytes read = current pointer position (after writing all the bytes) - pointer start
    return u8_data_out - (uint8_t*)out_buf;
}

bool FAT_readdir(DISK *disk, FAT_File* file, FAT_direntry* out)
{
    return FAT_read(disk, file, sizeof(FAT_direntry), out) == sizeof(FAT_direntry);
}

void FAT_close(FAT_File* file)
{
    if(file->handle == ROOT_DIRECTORY_HANDLE)
    {
        file->pos = 0;
        g_Data->root_directory.current_cluster = g_Data->root_directory.first_cluster;
    }
    else
    {
        g_Data->open_files[file->handle].opened = false;
    }

}

bool FAT_FindFile(DISK* disk, FAT_File* file, const char *name, FAT_direntry* entry_out) {
    char fat_name[11];
    FAT_direntry entry;

    // convert from name to fat name
    memset(fat_name, ' ', sizeof(fat_name));
    const char* ext = strchr(name, '.');

    if (ext == NULL)
        ext = name + 11;

    for(int i = 0; i < 8 && name[i] && name + i < ext; i++)
    {
        fat_name[i] = toupper(name[i]);
    }

    if (ext != NULL) {
        for (int i = 0; i < 3 && ext[i + 1]; i++) {
            fat_name[i + 8] = toupper(ext[i + 1]);
        }
    }

    while(FAT_readdir(disk, file, &entry))
    {
        if(memcmp(fat_name, entry.name, 11) == 0)
        {
            *entry_out = entry;
            return true;
        }
    }

    return false; // file not found (probably does not exist)
}

FAT_File* FAT_Open(DISK* disk, const char* path)
{
    char name[MAX_PATH_SIZE];

    // ignore leading slash
    if (path[0] == '/')
        path++;

    FAT_File* current = &g_Data->root_directory.public;

    while(*path) {
        // extract filename from path
        bool is_last = false;
        const char* delim = strchr(path, '/');
        if (delim != NULL)
        {
            memcpy(name, path, delim - path);
            name[delim - path + 1] = '\0';
            path = delim + 1;
        }
        else
        {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            is_last = true;
        }


        // find directory entry in current directory
        FAT_direntry entry;
        if(FAT_FindFile(disk, current, name, &entry))
        {
            FAT_close(current);

            // check if directory
            if(!is_last && (entry.attributes & FAT_ATTR_DIRECTORY) == 0)
            {
                printf("FAT: %s is not a directory\r\n", name);
                return NULL;
            }

            current = FAT_open_entry(disk, &entry);
        }
        else
        {
            FAT_close(current);
            printf("FAT: %s not found\r\n", name);
            return NULL;
        }
    }

    return current;
}
