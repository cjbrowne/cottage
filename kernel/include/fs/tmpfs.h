#pragma once

#include <fs/fs.h>
#include <lock/lock.h>
#include <scheduler/event.h>

typedef struct {
    filesystem_t filesystem;
    uint64_t dev_id;
    uint64_t inode_counter;
} tmpfs_t;

typedef struct {
    resource_t resource;
    uint8_t* storage;
    size_t capacity;
} tmpfs_resource_t;

filesystem_t* tmpfs_init();
void tmpfs_populate(filesystem_t* self, vfs_node_t* node);
vfs_node_t* tmpfs_mount(filesystem_t* self, vfs_node_t* parent, const char* name, vfs_node_t* source);
vfs_node_t* tmpfs_create_node(filesystem_t* self, vfs_node_t* parent, const char* name, int mode);
vfs_node_t* tmpfs_symlink(filesystem_t* self, vfs_node_t* parent, const char* dest, const char* target);
vfs_node_t* tmpfs_link(filesystem_t* self, vfs_node_t* parent, const char* dest, vfs_node_t* target);
void tmpfs_close(filesystem_t* self);

