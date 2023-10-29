#if 0 // disable compilation for now, we'll come back and finish implementing this later

#include <fs/fs.h>
#include <fs/ext2.h>
#include <panic.h>
#include <stdlib.h>

filesystem_t *ext2_init()
{
    ext2filesystem_t *ret = malloc(sizeof(ext2filesystem_t));

    ret->fs.instantiate = ext2_init;
    ret->fs.populate = ext2_populate;
    ret->fs.mount = ext2_mount;
    ret->fs.create_node = ext2_create_node;
    ret->fs.symlink = ext2_symlink;
    ret->fs.link = ext2_link;
    ret->fs.close = ext2_close;

    return (filesystem_t *)ret;
}

bool ext2_read_entry(ext2inode_t* node, ext2filesystem_t* fs, uint32_t inode)
{
    panic("todo: implement me", node, fs, inode);
}

void ext2_populate(filesystem_t *_self, vfs_node_t *node)
{
    ext2filesystem_t* self = _self;
    ext2inode_t* parent = malloc(sizeof(ext2inode_t));
    if (!ext2_read_entry(parent, self, node->resource->stat.inode))
    {
        klog("ext2", "Failed to read direntry");
        return;
    }

    uint64_t* buffer = malloc(parent->size_low);
    if(!ext2_read_inode(self, buffer, 0, parent->size_low))
    {
        klog("ext2", "Failed to read inode");
        free(buffer);
        return;
    }

    for(uint32_t i = 0; i < parent->size_low;)
    {
        ext2_direntry_t* dir_entry = (ext2_direntry_t*)(uint64_t)buffer+i;

    }

    free(buffer);
}

__attribute__((unused))
vfs_node_t *ext2_mount(filesystem_t *self, vfs_node_t *parent, const char *name, vfs_node_t *source)
{
}

vfs_node_t *ext2_create_node(filesystem_t *self, vfs_node_t *parent, const char *name, int mode)
{
}

vfs_node_t *ext2_symlink(filesystem_t *self, vfs_node_t *parent, const char *dest, const char *target)
{
}

vfs_node_t *ext2_link(filesystem_t *self, vfs_node_t *parent, const char *dest, vfs_node_t *target)
{
}

void ext2_close(filesystem_t *self)
{
}

#endif
