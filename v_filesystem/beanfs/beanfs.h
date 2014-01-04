//
//  beanfs.h
//  v_filesystem
//
//  Created by WONDER on 12/26/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#ifndef v_filesystem_beanfs_h
#define v_filesystem_beanfs_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SUPERBLOCKCOUNT 1
#define BOOTBLOCKCOUNT 1
#define FREE_DATABLOCKMG_BLOCK_COUNT 1
#define FREE_INODESMG_BLOCK_COUNT 1
#define FREE_SPACEMG_BLOCK_COUNT 2

#define BLOCK_SIZE 512
// first inode is for root
#define BEANFS_ROOT_INODE_OFFSET
#define FREE_DATABLOCKS_LIST_SIZE 100
#define FREE_INODES_LIST_SIZE 100


// structure of the superblock on virtual
// using logical block address from 0 to user defined number
struct beanfs_super_block {
    
    uint32_t s_blocks_count;
    uint32_t s_inodes_count;
    uint32_t s_datablocks_count;
    uint32_t s_free_datablocks_count;
    uint32_t s_free_inodes_count;
    
    uint32_t s_free_datablocksmg_block;              // block position for free lists group; boot, superblock, free_datablock_spacemg_block
    uint32_t s_free_inodesmg_block;                  // then come freeinodes space manage block
    
    uint32_t s_first_data_block;
    uint32_t s_first_inode_block;
    
    uint32_t s_mtime;                             // modified timestamp
    uint32_t s_birthtime;                         // created timestamp
};


struct free_datablocks_group {
    int8_t top;
    uint32_t list[FREE_DATABLOCKS_LIST_SIZE];
};

struct free_inodes_group {
    int8_t top;
    uint32_t list[FREE_INODES_LIST_SIZE];
};

struct free_blocks_group {
    struct free_datablocks_group data_group;
    struct free_inodes_group inode_group;
};

// structure of superblock in memomry
struct beanfs_sb_info {
    
    uint32_t s_blocks_count;                // total disk blocks 512byte per block
    uint32_t s_datablocks_count;
    uint32_t s_inodes_count;
    uint32_t s_free_datablocks_count;
    uint32_t s_free_inodes_count;
    
    uint32_t s_free_datablocksmg_block;              // block position for free lists group; boot, superblock, free_datablock_spacemg_block
    uint32_t s_free_inodesmg_block;                  // then come freeinodes space manage block
    struct free_datablocks_group s_free_datablocks_group;
    struct free_inodes_group s_free_inodes_group;
    
    uint32_t s_first_data_block;             // first data block
    uint32_t s_first_inode_block;            // first inode
    
    uint8_t s_free_blocks_list_lock;         // 0 stand for unlock and 1 stand for unlock
    uint8_t s_free_inodes_list_lock;
    
    uint8_t s_ismodified;                    // flag for wheather modified
    uint32_t s_mtime;                        // modified time stamp
    uint32_t s_birthtime;
};


#define BEANFS_NDADDR 4
#define BEANFS_NINDIR 2

// structure of inode on v_disk
struct beanfs_inode {
    
    uint16_t i_mode;                      // permission
    uint16_t i_links;                     // file link count
    uint32_t i_uid;                       // file onwer
    uint32_t i_gid;                       // file owner group
    
    uint32_t i_atime;                     // last access time
    uint32_t i_mtime;                     // last modify time
    uint32_t i_ctime;                     // last inode changetime
    uint32_t i_birthtime;                 // creation time
    
    uint32_t i_size;                      // file bytes count
    uint32_t i_blocks;                    // actual blocks held
    
    struct {
        uint32_t d_addr[BEANFS_NDADDR];     // direct addr; number in data_blocks
        uint32_t id_addr[BEANFS_NINDIR];     // first is 1st direct and second is 2th indirect
    } i_addr;
    
};

// structure of inode in memory
struct beanfs_inode_info {
    
    uint32_t i_ino;
    uint8_t i_access_count;                 // access count
    uint8_t i_links;                        // file link count
    uint16_t i_mode;                        // permission
    uint16_t i_uid;                         // file onwer
    uint16_t i_gid;                         // file owner group
    
    uint32_t i_atime;                       // last access time
    uint32_t i_mtime;                       // last modify time
    uint32_t i_ctime;                       // last inode changetime
    uint32_t i_birthtime;                   // creation time
    
    uint32_t i_size;                        // file bytes count
    uint32_t i_blocks;                      // actual blocks held
    
    struct {
        uint32_t d_addr[BEANFS_NDADDR];        // direct addr; offset in data blocks
        uint32_t id_addr[BEANFS_NINDIR];        // first is 1st direct and second is 2th indirect
    } i_addr;
    
    uint8_t i_lock;                                 // 0 : unlocked and 1 : locked
};

#define MAX_ENTRY_NAME 20
#define MAX_ENTRY 18
struct beanfs_dir_entry {
    
    uint32_t d_ino;                             // 0; inode number in inodes
    char d_name[MAX_ENTRY_NAME];                // 5; name_length
    char d_file_type;                           // 25; file type 'd' or 's' or '-'
};                                              // 26; bytes

struct beanfs_dir {
    uint8_t len;
    struct beanfs_dir_entry entrys[MAX_ENTRY];
};

/* function for call
 */

extern void create_raw_sb(struct beanfs_super_block *sbp, uint32_t blocks);
extern int read_superblock(struct beanfs_super_block *sbp, FILE *v_device);
extern int write2block(const void *buffer, long dst_block, size_t size, size_t count, FILE *vdevice);
extern int init_beanfs(uint32_t blocks, FILE *virtual_device);
extern int read_block(void *buffer, long dst_block, size_t size , size_t count, FILE *v_device);
extern int beanfs_read_inode(struct beanfs_sb_info *sb_info_p, struct beanfs_inode *inode_p, uint32_t inode_addr, FILE *v_device);


#endif