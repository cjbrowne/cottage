#pragma once

#include <lock/lock.h>
#include <resource/resource.h>

// number of filesystems that are included in the kernel source tree
// increase this if you add a new filesystem, or else get bugs
#define KERNEL_FILESYSTEM_COUNT 3

// the indices into the filesystems[] array for each of the kernel-included
// filesystems.  Modular filesystems are placed after the baked in ones.
#define FS_TMPFS 0
#define FS_DEVTMPFS 1
#define FS_EXT2 2

typedef struct filesystem_s filesystem_t;
typedef struct vfs_node_s vfs_node_t;

typedef struct filesystem_s {
    filesystem_t* (*instantiate)(void);
    void (*populate)(filesystem_t* self, vfs_node_t* node);
    vfs_node_t* (*mount)(filesystem_t* self, vfs_node_t* parent, const char* name, vfs_node_t* source);
    vfs_node_t* (*create_node)(filesystem_t* self, vfs_node_t* parent, const char* name, int mode);
    vfs_node_t* (*symlink)(filesystem_t* self, vfs_node_t* parent, const char* dest, const char* target);
    vfs_node_t* (*link)(filesystem_t* self, vfs_node_t* parent, const char* dest, vfs_node_t* target);
    void (*close)(filesystem_t* self);
} filesystem_t;

typedef struct vfs_node_s {
    vfs_node_t* mountpoint;
    vfs_node_t* redir;
    resource_t* resource;
    filesystem_t* filesystem;
    const char* name;
    vfs_node_t* parent;
    vfs_node_t* children;

    // -1 means "not a directory", 0 is a directory with 0 children
    int children_count; 

    const char* symlink_target;
} vfs_node_t;

void fs_init();
vfs_node_t* vfs_create_node(filesystem_t* filesystem, vfs_node_t* parent, const char* name, bool dir);
void vfs_add_child(int* num_children, vfs_node_t** children, vfs_node_t* new_child);

// todo: extern these?
// extern lock_t vfs_lock;
// extern vfs_node_t* vfs_root;
extern filesystem_t** filesystems;
