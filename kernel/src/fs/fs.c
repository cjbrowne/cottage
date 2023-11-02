#include <fs/fs.h>
#include <fs/tmpfs.h>
#include <fs/devtmpfs.h>
#include <lock/lock.h>
#include <klog/klog.h>
#include <debug/debug.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

lock_t vfs_lock;
vfs_node_t* vfs_root;
filesystem_t** filesystems;
size_t num_filesystems;

vfs_node_t* vfs_create_node(filesystem_t* filesystem, vfs_node_t* parent, const char* name, bool dir)
{
    vfs_node_t* node = malloc(sizeof(vfs_node_t));

    // copy the name into the name field, so we know this node "owns" its own memory
    node->name = malloc(strlen(name)+1);
    memcpy(node->name, name, strlen(name));

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

void vfs_add_child(vfs_node_t* parent, vfs_node_t* new_child)
{
    // the last node is already pre-allocated for us, so just copy the new child in
    parent->children[parent->children_count] = *new_child;
    parent->children_count++;
    // pre-allocate the next child
    parent->children = realloc(parent->children, sizeof(vfs_node_t*) * (parent->children_count + 1));
}

void fs_init()
{
   vfs_root = vfs_create_node(NULL, NULL, "", false);

   filesystems = malloc(sizeof(filesystem_t*) * KERNEL_FILESYSTEM_COUNT); 
   filesystems[FS_TMPFS] = tmpfs_create();

   filesystems[FS_DEVTMPFS] = devtmpfs_create();

   num_filesystems += 2; 
   // todo: finish implementing ext2 support 
   //filesystems[FS_EXT2] = ext2_init();
}

vfs_node_t* reduce_node(vfs_node_t* node, bool follow_symlinks)
{
    if (node->redir != 0)
    {
        return reduce_node(node->redir, follow_symlinks);
    }
    if (node->mountpoint != 0)
    {
        return reduce_node(node->mountpoint, follow_symlinks);
    }
    if (follow_symlinks && node->symlink_target != NULL && strlen(node->symlink_target) != 0)
    {
        path2node_return_t ret = path2node(node->parent, node->symlink_target);
        vfs_node_t* next_node = ret.current;
        // we don't use the basename
        free(ret.basename);
        if (next_node == NULL)
        {
            return NULL;
        }
        return reduce_node(next_node, follow_symlinks);
    }
    
    return node;
}

// expects child_name to be zero-terminated string containing only the 
// specific child we are looking for.  Do not pass in a subpath, as we will
// not split it! (this behaviour may change in future versions)
vfs_node_t* node_get_child(vfs_node_t* node, const char* child_name)
{
    klog("fs", "node_get_child node=%x child_name=%s children=%d",
        node,
        child_name,
        node->children_count
    );
    // very simple implementation so we just do a linear search,
    // in future versions we may implement some kind of hash table for node
    // children, to speed things up when we're looking through large directories
    for(int i = 0; i < node->children_count; i++)
    {
        // skip entries where the names are different lengths (stops us matching prefixes by accident)
        if(strlen(node->children[i].name) != strlen(child_name)) continue;
        if(strncmp(node->children[i].name, child_name, strlen(child_name)) == 0)
        {
            return &node->children[i];
        }
    }
    return NULL;
}

path2node_return_t path2node(vfs_node_t* parent, const char* path)
{
    if(strlen(path) == 0)
    {
        // todo: set errno(ENOENT)
        return (path2node_return_t){
            NULL,
            NULL,
            NULL 
        };
    }

    uint64_t index = 0;
    vfs_node_t* current_node = reduce_node(parent, false);

    if (path[index] == '/')
    {
        // it's an absolute path, so start from vfs_root instead
        current_node = reduce_node(vfs_root, false);
        while(path[index] == '/') // eat up all extraneous leading `/` characters
        {
            if(index == strlen(path) - 1)
            {
                // if we only have `/` characters, we're essentially at the root
                return (path2node_return_t){current_node, current_node, NULL};
            }
            index++;
        }
    }

    while(1)
    {
        // warning! this pointer sometimes gets returned, do not free() in those cases
        // - we expect the pointer to be free'd when it is copied into a structure
        char* path_elem = malloc(strlen(path));
        size_t i = 0;
        // get the next component from the path string
        while(index < strlen(path) && path[index] != '/')
        {
            path_elem[i++] = path[index++];
        }

        // eat up all the extraneous `/` separators
        while(index < strlen(path) && path[index] == '/') 
        {
            index++;
        }

        bool last = index == strlen(path);

        current_node = reduce_node(current_node, false);


        vfs_node_t* new_node = node_get_child(current_node, path_elem);
        if(new_node == NULL)
        {
            // todo: set errno ENOENT
            if(last)
            {
                return (path2node_return_t){current_node, NULL, path_elem};
            }
            return (path2node_return_t){NULL, NULL, NULL};
        }

        new_node = reduce_node(new_node, false);

        if(last)
        {
            return (path2node_return_t){current_node, new_node, path_elem};
        }

        current_node = new_node;

        if(stat_is_lnk(current_node->resource->stat.mode))
        {
            path2node_return_t node = path2node(current_node->parent, current_node->symlink_target);
            if(node.current == NULL)
            {
                free(path_elem);
                return (path2node_return_t){NULL, NULL, NULL};
            }
            free(path_elem);
            continue;
        }

        if(!stat_is_dir(current_node->resource->stat.mode))
        {
            // todo: set errno ENOTDIR
            return (path2node_return_t){NULL, NULL, NULL};
        }

        free(path_elem);
    }

    // todo: set errno ENOENT
    return (path2node_return_t){NULL, NULL, NULL};
}

vfs_node_t* fs_symlink(vfs_node_t* parent, const char* dest, const char* target)
{
    path2node_return_t ret = path2node(parent, target);
    vfs_node_t* parent_of_tgt = ret.parent;
    vfs_node_t* target_node = ret.current;
    char* basename = ret.basename;

    if (target_node != NULL || parent_of_tgt != NULL)
    {
        // todo: set errno EEXIST
        // if we allocated a basename in the path2node function, we will need to free it here
        if(basename != NULL) free(basename);
        return NULL;
    }

    target_node = parent_of_tgt->filesystem->symlink(parent_of_tgt->filesystem, parent_of_tgt, dest, basename);
    free(basename);

    vfs_add_child(parent_of_tgt, target_node);

    return target_node;
}

bool fs_mount(vfs_node_t* parent, const char* source, const char* target, hpr_fsid_t fs_identifier)
{
    if(fs_identifier > num_filesystems-1) 
    {
        klog("fs", "Mount failed: invalid filesystem identifier %d", fs_identifier);
        return false;
    }

    vfs_node_t* source_node = NULL;
    if (strlen(source) != 0)
    {
        path2node_return_t ret = path2node(parent, source);
        source_node = ret.current;
        free(ret.basename);
        if(source_node == NULL || stat_is_dir(source_node->resource->stat.mode))
        {
            klog("fs", "Mount failed: invalid source, or source is directory");
            return false;
        }
    }

    path2node_return_t ret = path2node(parent, target);
    vfs_node_t* target_node = ret.current;
    vfs_node_t* parent_of_tgt_node = ret.parent;
    char* basename = ret.basename;

    bool mounting_root = target_node == vfs_root;

    if (target_node == NULL || (!mounting_root && !stat_is_dir(target_node->resource->stat.mode)))
    {
        klog("fs", "Mount failed: target is not directory");
        return false;
    }


    filesystem_t* f_sys = filesystems[fs_identifier]->instantiate(filesystems[fs_identifier]);

    vfs_node_t* mount_node = f_sys->mount(f_sys, parent_of_tgt_node, basename, source_node);
    free(basename);

    target_node->mountpoint = mount_node;

    dir_create_dotentries(mount_node, parent_of_tgt_node);

    if(strlen(source) > 0)
    {
        klog("vfs", "Mounted %s to %s with filesystem %s", source, target, fs_name(fs_identifier));
    }
    else 
    {
        klog("vfs", "Mounted %s to %s", fs_name(fs_identifier), target);
    }
    return true;
}

vfs_node_t* internal_create(vfs_node_t* parent, const char* name, int mode)
{
    path2node_return_t ret = path2node(parent, name);
    vfs_node_t* parent_of_tgt_node = ret.parent;
    vfs_node_t* target_node = ret.current;
    char* basename = ret.basename;

    if(target_node != NULL)
    {
        // todo: set errno EEXIST
        free(basename);
        return NULL;
    }

    if(parent_of_tgt_node == NULL)
    {
        // todo: set errno ENOENT
        free(basename);
        return NULL;
    }

    target_node = parent_of_tgt_node->filesystem->create_node(parent_of_tgt_node->filesystem, parent_of_tgt_node, basename, mode);
    vfs_add_child(parent_of_tgt_node, target_node);
    if(stat_is_dir(target_node->resource->stat.mode))
    {
        dir_create_dotentries(target_node, parent_of_tgt_node);
    }

    free(basename);
    return target_node;
}

vfs_node_t* fs_create(vfs_node_t* parent, const char* name, int mode)
{
    lock_acquire(&vfs_lock);
    vfs_node_t* ret = internal_create(parent, name, mode);
    lock_release(&vfs_lock);
    return ret;
}

// create . and .. aliases to "current node" and "parent node", respectively
void dir_create_dotentries(vfs_node_t* node, vfs_node_t* parent)
{
    vfs_node_t* dot = vfs_create_node(node->filesystem, node, ".", false);
    vfs_node_t* dotdot = vfs_create_node(node->filesystem, node, "..", false);
    dot->redir = node;
    dotdot->redir = parent;
    vfs_add_child(node, dot);
    vfs_add_child(node, dotdot);
}

vfs_node_t* fs_get_node(vfs_node_t* parent, const char* path, bool follow_symlinks)
{
    path2node_return_t ret = path2node(parent, path);
    klog("fs", "path2node ret.current=%x ret.parent=%x ret.basename=%s", ret.current, ret.parent, ret.basename);
    free(ret.basename); // not used
    if(ret.current == NULL) return NULL;
    if (follow_symlinks)
    {
        return reduce_node(ret.current, true);
    }
    return ret.current;
}
