#pragma once

#include <time/timer.h>
#include <stdint.h>

#define STAT_DIRENT_NAME_LEN 1024

#define STAT_IFMT   0xf000
#define STAT_IFBLK  0x6000
#define STAT_IFCHR  0x2000
#define STAT_IFIFO  0x1000
#define STAT_IFREG  0x8000
#define STAT_IFDIR  0x4000
#define STAT_IFLNK  0xa000
#define STAT_IFSOCK 0xc000
#define STAT_IFPIPE 0x3000

typedef struct {
    uint64_t device;
    uint64_t inode;
    int mode;
    _Atomic int nlink;
    int uid;
    int gid;
    uint64_t rdev;
    uint64_t size;
    timespec_t accessed_time;
    timespec_t modified_time;
    timespec_t created_time;
    uint64_t block_size;
    uint64_t blocks;
} stat_t;

typedef struct {
    uint64_t inode;
    uint64_t offset;
    uint16_t reclen; // reclen?
    uint8_t type;
    uint8_t name[STAT_DIRENT_NAME_LEN];
} stat_dirent_t;

bool stat_is_dir(int mode);
bool stat_is_reg(int mode);
bool stat_is_lnk(int mode);
