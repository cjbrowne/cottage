#pragma once

#include <fs/fs.h>

typedef struct {
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t superuser_reserved;
    uint32_t unallocated_blocks;
    uint32_t unallocated_inodes;
    uint32_t sb_block;
    uint32_t block_size;
    uint32_t frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_written_time;
    uint16_t mount_count;
    uint16_t mount_allowed;
    uint16_t signature;
    uint16_t fs_state;
    uint16_t error_response;
    uint16_t version_minor;
    uint32_t last_fsck;
    uint32_t forced_fsck;
    uint32_t os_id;
    uint32_t version_major;
    uint16_t user_id;
    uint16_t group_id;

    uint32_t first_inode;
    uint16_t inode_size;
    uint16_t sb_bgd; // which block group descriptor refers to the SB
    uint32_t optional_features;
    uint32_t required_features;
    uint32_t non_supported_features;
    uint64_t uuid[2]; // 128 bit UUID
    // todo: should I change these to char?
    uint64_t volume_name[2]; // 16-byte volume name
    uint64_t last_mnt_path[8]; // 64-byte path last mounted
} __attribute__((packed)) ext2superblock_t;

typedef struct {
    uint16_t permissions;
    uint16_t user_id;
    uint32_t size_low;
    uint32_t access_time;
	uint32_t created_time;
	uint32_t modified_time;
	uint32_t deleted_time;
	uint16_t group_id;
	uint16_t hard_link_count;
    uint32_t sector_count;
	uint32_t flags;
    // OS-specific data (can store anything we want in here)
	uint32_t oss1;
	uint32_t blocks[15];
    // primarily used for NFS
	uint32_t generation_num;
    // extended attribute block (file ACL)
    uint32_t eab;
    // this is either the upper 32 bits of the file size, or
    // the ACL if it's a directory
    uint32_t size_high;
    uint32_t frag_addr;
    // OS-specific data (can take bytes from this as needed for extra info)
    char oss2[12];
} __attribute__((packed)) ext2inode_t;

typedef struct {
    uint32_t inode_index;
    uint16_t entry_size;
    uint8_t name_length;
    uint8_t dir_type;
} __attribute__((packed)) ext2_direntry_t;

typedef struct {
    filesystem_t fs;
    uint64_t dev_id;
    ext2superblock_t* superblock;
    ext2inode_t* root_inode;
    uint64_t block_size;
    uint64_t frag_size;

    // reminder: bgd = block group descriptor
    uint64_t bgd_count;

    vfs_node_t* backing_device;
} ext2filesystem_t;

typedef struct {
    resource_t resource;
    ext2filesystem_t* filesystem;
} ext2_resource_t;

// standard FS functions
filesystem_t* ext2_init();
void ext2_populate(filesystem_t* self, vfs_node_t* node);
vfs_node_t* ext2_mount(filesystem_t* self, vfs_node_t* parent, const char* name, vfs_node_t* source);
vfs_node_t* ext2_create_node(filesystem_t* self, vfs_node_t* parent, const char* name, int mode);
vfs_node_t* ext2_symlink(filesystem_t* self, vfs_node_t* parent, const char* dest, const char* target);
vfs_node_t* ext2_link(filesystem_t* self, vfs_node_t* parent, const char* dest, vfs_node_t* target);
void ext2_close(filesystem_t* self);

// ext2 extras
bool ext2_read_entry(ext2inode_t* node, ext2filesystem_t* fs, uint32_t inode);
