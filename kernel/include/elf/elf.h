/* references:
    https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
    https://wiki.osdev.org/ELF
*/

#pragma once

#include <resource/resource.h>
#include <mem/pagemap.h>
#include <macro.h>

#include <stdint.h>

#define ELF_TYPE_RELOCATABLE     0x01
#define ELF_TYPE_EXECUTABLE      0x02
#define ELF_TYPE_SHARED          0x03
#define ELF_TYPE_CORE            0x04

#define ELF_MACHINE_UNSPECIFIED 0x0000
#define ELF_MACHINE_SPARC       0x0002
#define ELF_MACHINE_X86_32      0x0003
#define ELF_MACHINE_MIPS        0x0008
#define ELF_MACHINE_POWERPC     0x0014
#define ELF_MACHINE_ARM         0x0028
#define ELF_MACHINE_SUPERH      0x002A
#define ELF_MACHINE_IA64        0x0032
#define ELF_MACHINE_X86_64      0x003E
#define ELF_MACHINE_AARCH64     0x00B7
#define ELF_MACHINE_RISCV       0x00F3

#define ELF_FLAG_EXEC           0x01
#define ELF_FLAG_WRITE          0x02
#define ELF_FLAG_READ           0x04

#define ELF_IDENT_INDEX_CLASS           4
#define ELF_IDENT_INDEX_ENDIANNESS      5
#define ELF_IDENT_INDEX_HEADER_VERSION  6
#define ELF_IDENT_INDEX_OS_ABI          7

#define ELF_CLASS_32    1
#define ELF_CLASS_64    2

#define ELF_LITTLE_ENDIAN 1
#define ELF_BIG_ENDIAN    2

#define ELF_OS_ABI_SYSV     0

#define ELF_PTYPE_NULL          0
#define ELF_PTYPE_LOAD          1
#define ELF_PTYPE_DYNAMIC       2
#define ELF_PTYPE_INTERP        3
#define ELF_PTYPE_NOTE          4
#define ELF_PTYPE_SHLIB         5
#define ELF_PTYPE_PHDR          6
#define ELF_PTYPE_TLS           7

// aux table values - adjust as needed
#define ELF_AT_ENTRY 9
#define ELF_AT_PHDR 3
#define ELF_AT_PHENT 4
#define ELF_AT_PHNUM 5


typedef struct {
    uint8_t ident[8]; //bytes 0-7 define the type of ELF file
    RESERVE_BYTES(8); // bytes 8-15 are reserved
    uint16_t type;     // see ELF_TYPE macros
    uint16_t machine; // see ELF_MACHINE macros
    uint32_t version;
    uint64_t ptr_entry;         // the program entry point (vaddr)
    uint64_t program_header_offset;  // the program header table position
    uint64_t section_header_offset; // the section header table position
    uint32_t flags;  // see ELF_FLAG macros
    uint16_t header_size;       // total size of the ELF header
    uint16_t prog_header_entry_size;  
    uint16_t prog_header_num_entries; 
    uint16_t sect_header_entry_size;
    uint16_t sect_header_num_entries;
    uint16_t sect_header_names_idx;
} __attribute__((packed)) elf_header_t;

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t file_size;
    uint64_t mem_size;
    uint64_t align;
} __attribute__((packed)) elf_program_header_t;

typedef struct {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addr_align;
    uint64_t entity_size;
} __attribute__((packed)) elf_section_header_t;


// as this is sometimes used to populate an aux vector,
// the following translations may be helpful:
// AT_ENTRY -> elf_info_t.entry
// AT_PHDR -> elf_info_t.headers_ptr
// AT_PHENT -> elf_info_t.prog_header_entry_size
// AT_PHNUM -> elf_info_t.num_headers
typedef struct {
    uint64_t entry;
    uint64_t prog_headers_ptr;
    uint64_t prog_header_entry_size;
    uint64_t num_headers;
    char* ld_path;
} elf_info_t;

/**
 * @brief loads an ELF object from resource into the provided pagemap
 * 
 * @param pagemap where to load the ELF object
 * @param resource  where to find the ELF object to be loaded
 * @param base the base address in virtual memory at which to load the ELF object
 * @param ret if non-NULL, will be populated with information about the ELF object that was loaded
 * @return true if the load succeeds
 * @return false on error
 */
bool elf_load(pagemap_t* pagemap, resource_t* resource, uint64_t base, elf_info_t* ret);
