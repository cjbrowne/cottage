#include <fs/fs.h>
#include <fs/tmpfs.h>
#include <fs/devtmpfs.h>
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

void vfs_add_child(int* num_children, vfs_node_t** children, vfs_node_t* new_child)
{
    // the last node is already pre-allocated for us
    children[*num_children] = new_child;
    // pre-allocate the next child
    (*num_children)++;
    *children = realloc(*children, sizeof(vfs_node_t*) * (*num_children)+1);
}

void fs_init()
{
   vfs_root = vfs_create_node(NULL, NULL, "", false);

   filesystems = malloc(sizeof(filesystem_t*) * KERNEL_FILESYSTEM_COUNT); 
   filesystems[FS_TMPFS] = tmpfs_init();

   filesystems[FS_DEVTMPFS] = devtmpfs_init();
//  filesystems[FS_EXT2] = ext2fs_init();
}

