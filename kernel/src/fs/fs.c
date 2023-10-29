#include <fs/fs.h>
#include <fs/tmpfs.h>
#include <lock/lock.h>
#include <stdlib.h>

#include <stdbool.h>

lock_t vfs_lock;
vfs_node_t* vfs_root;
filesystem_t** filesystems;

vfs_node_t* vfs_create_node(filesystem_t* filesystem, vfs_node_t* parent, const char* name, bool dir)
{
    vfs_node_t* node = malloc(sizeof(vfs_node_t));
    node->name = name;
    node->parent = parent;
    node->mountpoint = NULL;
    node->redir = NULL;
    node->children = NULL;
    node->children_count = -1;
    node->resource = malloc(sizeof(resource_t));
    node->filesystem = filesystem;

    if(dir)
    {
        // pre-allocate the first child -- adding children should always
        // populate the current node, and pre-allocate the next node 
        node->children = malloc(sizeof(vfs_node_t));
        node->children_count = 0;
    }


    return node;
}

void fs_init()
{
   vfs_root = vfs_create_node(NULL, NULL, "", false);

   filesystems = malloc(sizeof(filesystem_t*) * KERNEL_FILESYSTEM_COUNT); 
   filesystems[FS_TMPFS] = tmpfs_init();

//  filesystems[FS_DEVTMPFS] = devtmpfs_create();
//  filesystems[FS_EXT2] = ext2fs_create();
}

