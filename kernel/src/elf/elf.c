// first-party headers
#include <elf/elf.h>
#include <mem/pmm.h>
#include <mem/align.h>
#include <mem/mmap.h>

// standard headers
#include <string.h>
#include <stdlib.h>

bool elf_load(pagemap_t* pagemap, resource_t* resource, uint64_t base, elf_info_t* ret)
{
    elf_header_t header;

    // read the header
    int64_t bytes_read = resource->read(resource, NULL, &header, 0, sizeof(header));
    if(bytes_read < 0)
    {
        return false;
    }
    // check for magic header: 0x7F, 'E', 'L', 'F'
    if(memcmp(&header.ident, "\177ELF", 4) != 0)
    {
        // not a valid ELF file
        return false;
    }

    if(     header.ident[ELF_IDENT_INDEX_CLASS] != ELF_CLASS_64
        ||  header.ident[ELF_IDENT_INDEX_ENDIANNESS] != ELF_LITTLE_ENDIAN
        ||  header.ident[ELF_IDENT_INDEX_OS_ABI] != ELF_OS_ABI_SYSV
        ||  header.machine != ELF_MACHINE_X86_64
    )
    {
        // unsupported architecture / ELF format / ABI
        return false;
    }

	if(ret != NULL) {
        *ret = (elf_info_t){
            .entry = base + header.ptr_entry,
            .prog_headers_ptr = 0,
            .prog_header_entry_size = sizeof(elf_program_header_t),
            .num_headers = header.prog_header_num_entries,
            .ld_path = NULL,
        };
    }

    for(uint64_t i = 0; i < header.prog_header_num_entries; i++)
    {
        elf_program_header_t program_header;
        resource->read(resource, 0, &program_header, header.program_header_offset + sizeof(elf_program_header_t) * i, sizeof(elf_program_header_t));
        switch(program_header.type)
        {
            case ELF_PTYPE_INTERP:
                {
                    char* ld_path = malloc(program_header.file_size);
                    bytes_read = resource->read(resource, 0, ld_path, program_header.offset, program_header.file_size);
                    if (bytes_read < 0)
                    {
                        free(ld_path);
                        return false;
                    }
                    if(ret != NULL)
                        ret->ld_path = ld_path;
                }
                break;
            case ELF_PTYPE_PHDR:
                {
                    if (ret != NULL)
                        ret->prog_headers_ptr = base + program_header.vaddr;
                }
                break;
        }

		// skip any non-loadable segments (we don't need to load them... duh)
        if(program_header.type != ELF_PTYPE_LOAD)
        {
            continue;
        }
        uint64_t misalign = program_header.vaddr & (PAGE_SIZE - 1);
        uint64_t page_count = div_roundup(misalign + program_header.mem_size, PAGE_SIZE);
        uint64_t addr = (uint64_t)pmm_alloc(page_count);
        if (addr == 0)
        {
            // allocation failed
            return false;
        }
        uint64_t flags = MMAP_PROT_READ | MMAP_PROT_EXEC;
        if((program_header.flags & ELF_FLAG_WRITE) != 0)
        {
            flags |= MMAP_PROT_WRITE;
        }

        uint64_t virt = base + program_header.vaddr;
        uint64_t phys = addr;
        if(!mmap_map_range(pagemap, virt, phys, page_count * PAGE_SIZE, flags, MMAP_MAP_ANON))
        {
            // failed to map
            return false;
        }

        uint64_t buf = addr + misalign + HIGHER_HALF;
        bytes_read = resource->read(resource, 0, (void*)buf, program_header.offset, program_header.file_size);
        if(bytes_read < 0)
        {
            // read fail
            return false;
        }
    }

    return true;
}

