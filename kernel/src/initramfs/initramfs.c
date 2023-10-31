// first-party headers
#include <initramfs/initramfs.h>
#include <klog/klog.h>
#include <panic.h>
#include <mem/pmm.h>
#include <mem/align.h>
#include <fs/fs.h>

// third-party headers
#include <limine.h>

// standard headers
#include <stddef.h>
#include <string.h>

#define USTAR_HEADER_NAME_LEN 100

// checks max N chars in str and converts any octal digits found to
// int.  All other characters are ignored.
// so if you pass in e.g "0o644" it will convert 644 -> (int)420
uint64_t octal2int(char* str, size_t n)
{
    uint64_t ret = 0;
    for(size_t i = 0; str[i] != 0 && i < n; i++)
    {
        char val = str[i] - '0';
        if(val >= 0 && val <= 7)
        {
            ret *= 8;    // "shift" existing digits to the left
            ret += val;  // add the new digit at the right
        }
    }
    return ret;
}

void initramfs_init(struct limine_module_response* res)
{
    if (res == NULL || res->module_count < 1)
    {
        panic("No initramfs");
    }
    struct limine_file** modules = res->modules;

    void* initramfs_begin = modules[0]->address;
    uint64_t initramfs_size = modules[0]->size;

    klog("init", "initramfs_begin=%x initramfs_size=%d", initramfs_begin, initramfs_size);

    char name_override[USTAR_HEADER_NAME_LEN] = {0};

    ustarheader_t* current_header = initramfs_begin;

    while(1)
    {
        // this is not a USTar (Unix Standard TAR) header
        if(strncmp(current_header->signature, "ustar", 5) != 0)
        {
            break;
        }
        char name[USTAR_HEADER_NAME_LEN] = {0};
        if(strlen(name_override) == 0)
        {
            memcpy(name, current_header->name, USTAR_HEADER_NAME_LEN);
        }
        else
        {
            memcpy(name, name_override, USTAR_HEADER_NAME_LEN);
        }

        char* link_name = current_header->link_name;
        uint64_t size = octal2int(current_header->size, 12);
        uint64_t mode = octal2int(current_header->mode, 8);

        memset(name_override, 0, USTAR_HEADER_NAME_LEN);

        if (strncmp(name, "./", strlen(name)) == 0)
        {
            goto next;
        }

        switch(current_header->filetype)
        {
            case USTAR_FILETYPE_REGULAR_FILE : 
            {
                vfs_node_t* new_node = fs_create(vfs_root, name, mode | STAT_IFREG);
                if(new_node == NULL)
                {
                    panic("initramfs: failed to create file %s", name);
                }
                resource_t* resource = new_node->resource;
                void* buf = current_header+512;
                int64_t bytes_written = resource->write(resource, 0, buf, 0, size);
                if(bytes_written <= 0)
                {
                    panic("initramfs: failed to write %s", name);
                }
            }
            break;
            case USTAR_FILETYPE_HARD_LINK    : 
            // not handled
            break;
            case USTAR_FILETYPE_SYM_LINK     : 
            {
                vfs_node_t* sl = fs_symlink(vfs_root, link_name, name);
                if(sl == NULL)
                {
                    panic("initramfs: failed to create symlink %s", name);
                }
            }
            break;
            case USTAR_FILETYPE_CHAR_DEVICE  : 
            // not handled
            break;
            case USTAR_FILETYPE_BLOCK_DEVICE : 
            // not handled
            break;
            case USTAR_FILETYPE_DIRECTORY    : 
            {
                if (fs_create(vfs_root, name, mode | STAT_IFDIR) == NULL)
                {
                    panic("initramfs: failed to create directory %s", name);
                }
            }
            break;
            case USTAR_FILETYPE_FIFO         : 
            // not handled
            break;
            case USTAR_FILETYPE_GNU_LONG_PATH: 
            {
            	// don't allow ridiculously long file names because wtf, why would
                // you want a 64K file name... you're a weirdo.  Stop it.  Get some help.
                if (size >= 65536)
                {
                    panic("initramfs: long file name exceeds 65535 characters");
                }
                memcpy(name_override, current_header+512, size);
            }
            break;
        }

next:
		// mark the memory that limine allocated for this file as free
        pmm_free(current_header - HIGHER_HALF, (512 + align_up(size, 512))/PAGE_SIZE);
        // increment header by 1 page (header takes up 1 page) plus however many
        // pages this file took up
        current_header += (512 + align_up(size, 512));
    }
    klog("init", "Initramfs loaded"); 
}

