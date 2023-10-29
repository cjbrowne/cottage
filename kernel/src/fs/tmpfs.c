#include <fs/fs.h>
#include <fs/tmpfs.h>
#include <stat/stat.h>
#include <string.h>
#include <panic.h>

#include <stdlib.h>


filesystem_t* tmpfs_init()
{
    tmpfs_t* ret = malloc(sizeof (tmpfs_t));
    ret->filesystem.instantiate = tmpfs_init;
    ret->filesystem.populate = tmpfs_populate;
    ret->filesystem.mount = tmpfs_mount;
    ret->filesystem.create_node = tmpfs_create_node;
    ret->filesystem.symlink = tmpfs_symlink;
    ret->filesystem.link = tmpfs_link;
    ret->filesystem.close = tmpfs_close;
    return (filesystem_t*)ret;
}

void tmpfs_close(__attribute__((unused)) filesystem_t* self)
{
    // todo: implement a thread-safe `close` function for tmpfs?
    panic("TMPFS should never be closed!");
}

void tmpfs_populate(__attribute__((unused)) filesystem_t* self, __attribute__((unused)) vfs_node_t* node)
{
    // body left deliberately empty (populate is a no-op for tmpfs)
}

// todo: why is `source` unused?  VINIX doesn't use it, but surely we need to mount the source node into the node tree
// in this function?  Unless I've misunderstood something, this may be a bug we have accidentally ported from VINIX
// which wouldn't surprise me, as mounting a sub-filesystem into a tmpfs is an unusual usage pattern so I would
// expect this code to be less thoroughly tested
// if it is a bug, feel free to submit a PR to fix it!
vfs_node_t* tmpfs_mount(filesystem_t* self, vfs_node_t* parent, const char* name, __attribute__((unused)) vfs_node_t* source)
{
    ((tmpfs_t*)self)->dev_id = resource_create_dev_id(); 
    return self->create_node(self, parent, name, 0644 | STAT_IFDIR);
}

vfs_node_t* tmpfs_create_node(filesystem_t* _self, vfs_node_t* parent, const char* name, int mode)
{
    tmpfs_t* self = (tmpfs_t*)_self;
    vfs_node_t* new_node = vfs_create_node(_self, parent, name, stat_is_dir(mode));

    tmpfs_resource_t* new_resource = malloc(sizeof(tmpfs_resource_t));

    new_resource->storage = 0;
    new_resource->resource.refcount = 1;

    if(stat_is_reg(mode))
    {
        new_resource->capacity = 4096;
        new_resource->storage = malloc(new_resource->capacity);
        new_resource->resource.can_mmap = true;
    }

    new_resource->resource.stat.size = 0;
    new_resource->resource.stat.blocks = 0;
    new_resource->resource.stat.block_size = 512;
    new_resource->resource.stat.device = self->dev_id;
    new_resource->resource.stat.inode = self->inode_counter++;
    new_resource->resource.stat.mode = mode;
    new_resource->resource.stat.nlink = 1;

    new_resource->resource.stat.accessed_time = realtime_clock;
    new_resource->resource.stat.created_time = realtime_clock;
    new_resource->resource.stat.modified_time = realtime_clock;
    
    new_node->resource = (resource_t*)new_resource;

    return new_node;
}

vfs_node_t* tmpfs_symlink(filesystem_t* _self, vfs_node_t* parent, const char* dest, const char* target)
{
    tmpfs_t* self = (tmpfs_t*)_self;
    vfs_node_t* new_node = vfs_create_node(_self, parent, target, false);
    tmpfs_resource_t* new_resource = malloc(sizeof(tmpfs_resource_t));

    new_resource->storage = 0;
    new_resource->resource.refcount = 1;

    new_resource->resource.stat.size = strlen(target);
    new_resource->resource.stat.blocks = 0;
    new_resource->resource.stat.block_size = 512;
    new_resource->resource.stat.device = self->dev_id;
    new_resource->resource.stat.inode = self->inode_counter++;
    new_resource->resource.stat.mode = STAT_IFLNK | 0777;
    new_resource->resource.stat.nlink = 1;

    new_resource->resource.stat.accessed_time = realtime_clock;
    new_resource->resource.stat.created_time = realtime_clock;
    new_resource->resource.stat.modified_time = realtime_clock;

    new_node->resource = (resource_t*) new_resource;

    new_node->symlink_target = dest;

    return new_node;
}

vfs_node_t*tmpfs_link(filesystem_t* self, vfs_node_t* parent, const char* dest, vfs_node_t* target)
{
    vfs_node_t* new_node = vfs_create_node(self, parent, dest, false);

    atomic_fetch_add(&target->resource->refcount, 1);

    new_node->resource = target->resource;
    new_node->children = target->children;

    return new_node;
}
