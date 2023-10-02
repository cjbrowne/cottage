#pragma once

// reminder: segment:offset is reversed in C,
// so addresses will look like "OOOOSSSS",
// rather than SSSSOOOO
// (I think the tutorial video guy is also confused by this)

// the NULL address is at the beginning of memory (0x00000000)
#define NULL ((void*)0)

// 0x00000000 - 0x000003FF - interrupt vector table
// 0x00000400 - 0x000004FF - BIOS data area

#define MEMORY_MIN  0x00000500
#define MEMORY_MAX  0x00080000

// 0x00000500 - 0x00010500 - FAT driver
#define MEMORY_FAT_ADDR ((void*)0x00020000)
#define MEMORY_FAT_SIZE 0x00010000

// 0x00020000 - 0x00030000 - stage2

// 0x00030000 - 0x00080000 - free (can allocate here)
#define MEMORY_LOAD_KERNEL ((void*)0x00030000)
#define MEMORY_LOAD_SIZE 0x00010000

// 0x00080000 - 0x0009FFFF - Extended BIOS data area
// 0x000A0000 - 0x000C7FFF - Video
// 0x000C8000 - 0x000FFFFF - BIOS

// place kernel after all the BIOS pages
#define MEMORY_KERNEL_ADDR ((void*)0x00100000)