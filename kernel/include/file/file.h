#pragma once

#include <resource/resource.h>
#include <lock/lock.h>
#include <fs/fs.h>
#include <stat/stat.h>

#include <stdint.h>
#include <stddef.h>

typedef struct file_handle_s {
    lock_t lock;
    resource_t* resource;
    vfs_node_t* node;
    size_t refcount;
    int64_t location; // fseek() internal state
    uint64_t flags;

	// is this something which can be treated as a directory?
    bool can_dirlist;
    stat_dirent_t** dirlist;
    size_t dirlist_count;
    uint64_t dirlist_index;

} file_handle_t;

typedef struct file_descriptor_s {
    file_handle_t* handle;
    int flags;
} file_descriptor_t;

