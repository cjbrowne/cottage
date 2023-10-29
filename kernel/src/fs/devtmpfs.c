#include <stdlib.h>
#include <fs/fs.h>
#include <fs/devtmpfs.h>
#include <string.h>
#include <panic.h>

uint64_t devtmpfs_dev_id = 0;
uint64_t devtmpfs_inode_counter = 0;
vfs_node_t* devtmpfs_root = NULL;

filesystem_t* devtmpfs_init()
{
    devtmpfs_t* ret = malloc(sizeof (devtmpfs_t));
    ret->filesystem.instantiate = devtmpfs_init;
    ret->filesystem.populate = devtmpfs_populate;
    ret->filesystem.mount = devtmpfs_mount;
    ret->filesystem.create_node = devtmpfs_create_node;
    ret->filesystem.symlink = devtmpfs_symlink;
    ret->filesystem.link = devtmpfs_link;
    ret->filesystem.close = devtmpfs_close;
    return (filesystem_t*)ret;
}

void devtmpfs_populate(__attribute__((unused)) filesystem_t* self, __attribute__((unused)) vfs_node_t* node)
{

}

// I honestly don't even know what the `source` parameter is for at this point
vfs_node_t* devtmpfs_mount(filesystem_t* _self, vfs_node_t* parent, const char* name, __attribute__((unused)) vfs_node_t* source)
{
    devtmpfs_t* self = (devtmpfs_t*) _self;

    if (devtmpfs_dev_id == 0)
    {
        devtmpfs_dev_id = resource_create_dev_id();
    }

    if (devtmpfs_root == NULL)
    {
        devtmpfs_root =  self->filesystem.create_node(_self, parent, name, 0644 | STAT_IFDIR);
    }

    return devtmpfs_root;
}

vfs_node_t* devtmpfs_create_node(filesystem_t* _self, vfs_node_t* parent, const char* name, int mode)
{
    vfs_node_t* new_node = vfs_create_node(_self, parent, name, stat_is_dir(mode));

    devtmpfs_resource_t* new_resource = malloc(sizeof(devtmpfs_resource_t));

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
    new_resource->resource.stat.device = devtmpfs_dev_id;
    new_resource->resource.stat.inode = devtmpfs_inode_counter++;
    new_resource->resource.stat.mode = mode;
    new_resource->resource.stat.nlink = 1;

    new_resource->resource.stat.accessed_time = realtime_clock;
    new_resource->resource.stat.created_time = realtime_clock;
    new_resource->resource.stat.modified_time = realtime_clock;
    
    new_node->resource = (resource_t*)new_resource;

    return new_node;
}

vfs_node_t* devtmpfs_symlink(filesystem_t* _self, vfs_node_t* parent, const char* dest, const char* target)
{
    vfs_node_t* new_node = vfs_create_node(_self, parent, target, false);
    devtmpfs_resource_t* new_resource = malloc(sizeof(devtmpfs_resource_t));

    new_resource->storage = 0;
    new_resource->resource.refcount = 1;

    new_resource->resource.stat.size = strlen(target);
    new_resource->resource.stat.blocks = 0;
    new_resource->resource.stat.block_size = 512;
    new_resource->resource.stat.device = devtmpfs_dev_id;
    new_resource->resource.stat.inode = devtmpfs_inode_counter++; 
    new_resource->resource.stat.mode = STAT_IFLNK | 0777;
    new_resource->resource.stat.nlink = 1;

    new_resource->resource.stat.accessed_time = realtime_clock;
    new_resource->resource.stat.created_time = realtime_clock;
    new_resource->resource.stat.modified_time = realtime_clock;

    new_node->resource = (resource_t*) new_resource;

    new_node->symlink_target = dest;

    return new_node;
}

vfs_node_t* devtmpfs_link(filesystem_t* self, vfs_node_t* parent, const char* dest, vfs_node_t* target)
{
    vfs_node_t* new_node = vfs_create_node(self, parent, dest, false);

    atomic_fetch_add(&target->resource->refcount, 1);

    new_node->resource = target->resource;
    new_node->children = target->children;

    return new_node;
}

void devtmpfs_close(filesystem_t* self)
{
    panic("devtmpfs_close not implemented %x", self);
}

void devtmpfs_add_device(resource_t* device, const char* name)
{
    vfs_node_t* new_node = vfs_create_node(filesystems[FS_DEVTMPFS], devtmpfs_root, name, false);

    new_node->resource = device;
    new_node->resource->stat.device = devtmpfs_dev_id;
    new_node->resource->stat.inode = devtmpfs_inode_counter++;
    new_node->resource->stat.nlink = 1;

    new_node->resource->stat.accessed_time = realtime_clock;
    new_node->resource->stat.created_time = realtime_clock;
    new_node->resource->stat.modified_time = realtime_clock;

    vfs_add_child(&devtmpfs_root->children_count, &devtmpfs_root->children, new_node);
}
