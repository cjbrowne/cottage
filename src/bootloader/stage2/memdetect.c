#include "memdetect.h"
#include "x86.h"
#include "stdio.h"

memory_region g_mem_regions[MAX_REGIONS];
int g_mem_region_count;

void memory_detect(memory_info* meminfo)
{
    E820MemoryBlock block;
    uint32_t continuation = 0;
    int ret;

    g_mem_region_count = 0;

    ret = x86_E820GetNextBlock(&block, &continuation);

    while(ret > 0 && continuation != 0)
    {

    g_mem_regions[g_mem_region_count].begin = block.base;
    g_mem_regions[g_mem_region_count].length = block.length;
    g_mem_regions[g_mem_region_count].acpi = block.acpi;
    g_mem_regions[g_mem_region_count].type = block.type;
    ++g_mem_region_count;

    printf("\t\t\t\tE820: base=0x%llx length=0x%llx type=0x%x\r\n", block.base, block.length, block.type);

    ret = x86_E820GetNextBlock(&block, &continuation);
    }
    // fill meminfo structure
    meminfo->regions = g_mem_regions;
    meminfo->region_count = g_mem_region_count;
}