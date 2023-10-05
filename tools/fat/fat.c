#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    uint8_t volume_id[4];
    uint8_t volume_label[11];
    uint8_t system_id[8];
} __attribute__((packed)) boot_sector;

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
} __attribute__((packed)) directory_entry;

// returns !0 on success, 0 on failure
uint8_t read_boot_sector(FILE *disk, boot_sector *target_boot_sector) {
    return (fread(target_boot_sector, sizeof(*target_boot_sector), 1, disk) > 0);
}

uint8_t read_sectors(FILE *disk, boot_sector bs, uint32_t lba, uint32_t count, void *buf) {
    // kinda janky use of lazy eval to shortcut error handling
    // will refactor later
    if (
            (fseek(disk, lba * bs.bytes_per_sector, SEEK_SET) == 0)
            &&
            (fread(buf, bs.bytes_per_sector, count, disk) == count)
            ) {
        return 1;
    } else {
        return 0;
    }
}

uint8_t read_fat(FILE *disk, boot_sector bs, uint8_t **FAT) {
    *FAT = malloc(bs.sectors_per_fat * bs.bytes_per_sector);
    return read_sectors(disk, bs, bs.reserved_sectors, bs.sectors_per_fat, *FAT);
}

uint8_t read_root_directory(FILE *disk, boot_sector bs, directory_entry **root_dir, uint32_t *directory_end) {
    uint32_t lba = bs.reserved_sectors + bs.sectors_per_fat * bs.fat_count;
    uint32_t size = sizeof(directory_entry) * bs.dir_entry_count;
    uint32_t sectors = (size / bs.bytes_per_sector);
    // round up sector count
    if (size % bs.bytes_per_sector != 0) {
        sectors++;
    }
    *directory_end = lba + sectors;
    *root_dir = malloc(sectors * bs.bytes_per_sector);
    return read_sectors(disk, bs, lba, sectors, *root_dir);
}

directory_entry *find_file(const char *name, boot_sector bs, directory_entry *root_dir) {
    for (uint32_t i = 0; i < bs.dir_entry_count; i++) {
        if (memcmp(name, root_dir[i].name, 11) == 0) {
            // found file!
            return &root_dir[i];
        }
    }

    return NULL; // file not found (probably does not exist)
}

uint8_t read_file(directory_entry *file_entry, uint8_t* FAT, FILE *disk, boot_sector bs, uint8_t *buf, uint32_t root_directory_end) {
    uint8_t ok = 1;
    uint16_t current_cluster = file_entry->first_cluster_low;
    do {
        uint32_t lba = root_directory_end + (current_cluster - 2) * bs.sectors_per_cluster;
        ok = read_sectors(disk, bs, lba, bs.sectors_per_cluster, buf);
        buf += bs.sectors_per_cluster * bs.bytes_per_sector;

        // do some fuckery to grab 12 bits at a time from the FAT table
        uint32_t fat_index = current_cluster * 3 / 2;
        if (current_cluster % 2 == 0) {
            current_cluster = (*(uint16_t*)(FAT + fat_index)) & 0x0FFF;
        } else {
            current_cluster = (*(uint16_t*)(FAT + fat_index)) >> 4;
        }

    } while (ok && current_cluster < 0xFF8);

    return ok;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE *disk = fopen(argv[1], "rb");

    if (disk == NULL) {
        fprintf(stderr, "Cannot open disk image %s\n", argv[1]);
        return -1;
    }

    boot_sector bs;

    if (!read_boot_sector(disk, &bs)) {
        fprintf(stderr, "could not read boot sector\n");
        return -1;
    }

    uint8_t* FAT;

    if (!read_fat(disk, bs, &FAT)) {
        fprintf(stderr, "Could not read FAT\n");
        return -1;
    }

    directory_entry *root_directory;
    uint32_t root_directory_end;

    if (!read_root_directory(disk, bs, &root_directory, &root_directory_end)) {
        fprintf(stderr, "Could not read root directory\n");
        return -1;
    }

    directory_entry *file_entry = find_file(argv[2], bs, root_directory);
    if (!file_entry) {
        fprintf(stderr, "File not found: %s\n", argv[2]);
        return -1;
    }


    uint8_t* buffer = malloc(file_entry->size + bs.bytes_per_sector);
    if(!read_file(file_entry, FAT, disk, bs, buffer, root_directory_end)) {
        fprintf(stderr, "Could not read file contents: %s\n", argv[2]);
        return -1;
    }

    for(size_t i = 0; i < file_entry->size; i++)
    {
        if(isprint(buffer[i])) {
            putc(buffer[i], stdout);
        } else {
            printf("<%02X>", buffer[i]);
        }
    }
    printf("\n");

    return 0;
}