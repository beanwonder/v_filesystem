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

#define BLOCK_SIZE 512
// first inode is for root
#define BEANFS_ROOT_INODE_OFFSET
#define FREE_BLOCKS_LIST_SIZE 60
#define FREE_INODES_LIST_SIZE 60

// structure of the superblock on virtual
// using logical block address from 0 to user defined number
struct beanfs_super_block {
    
    uint32_t s_blocks_count;
    uint32_t s_inodes_count;
    uint32_t s_data_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    
    uint32_t s_free_blocks_list_group[FREE_BLOCKS_LIST_SIZE];
    uint32_t s_free_inodes_list_group[FREE_INODES_LIST_SIZE];
    uint16_t blocks_list_top;              // number of blocks in free block list
    uint16_t inodes_list_top;              // number of inodes in free inode list
    
    uint32_t first_data_block;
    uint32_t first_inode;
    
    //uint16_t block_list_lock;             //0 stand for unlock and 1 stand for unlock
    //uint16_t inode_list_lock;
    
    uint32_t modify_time;                  // modified time stamp
};

// structure of superblock in memomry
struct beanfs_sb_info {
    
    char i_ftype;
    uint32_t s_blocks_count;
    uint32_t s_inodes_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    
    uint32_t s_free_blocks_list_group[60]; // array of blocks offset
    uint32_t s_free_inodes_list_group[60]; // array of inodes offset
    uint16_t blocks_list_top;              // number of blocks in free block list
    uint16_t inodes_list_top;              // number of inodes in free inode list
    
    uint32_t first_data_block;             // first data block
    uint32_t first_inode;                  // first inode
    
    uint16_t blocks_list_lock;             //0 stand for unlock and 1 stand for unlock
    uint16_t inodes_list_lock;
    
    uint16_t is_modified;                 // flag for wheather modified
    uint32_t modify_time;                  // modified time stamp
};


#define BEANFS_NDADDR 4
#define BEANFS_NINDIR 2

// structure of inode on v_disk
struct beanfs_inode {
    
    char i_file_type;
    uint16_t i_mode;                     // permission
    uint16_t i_links;                    // file link count
    uint32_t i_uid;                       // file onwer
    uint32_t i_gid;                       // file owner group
    
    uint32_t i_atime;                     // last access time
    uint32_t i_mtime;                     // last modify time
    uint32_t i_ctime;                     // last inode changetime
    uint32_t i_birthtime;                 // creation time
    
    uint32_t i_size;                      // file bytes count
    uint32_t i_blocks;                    // actual blocks held
    
    struct {
        uint32_t i_db_offset[BEANFS_NDADDR];     // direct addr; number in data_blocks
        uint32_t i_id_offset[BEANFS_NINDIR];     // first is 1st direct and second is 2th indirect
    } i_addr;
    
};

// structure of inode in memory
struct beanfs_inode_info {
    
    uint16_t i_access_count;             // access count
    
    uint16_t i_mode;                     // permission
    uint16_t i_links;                    // file link count
    uint32_t i_uid;                       // file onwer
    uint32_t i_gid;                       // file owner group
    
    uint32_t i_atime;                     // last access time
    uint32_t i_mtime;                     // last modify time
    uint32_t i_ctime;                     // last inode changetime
    uint32_t i_birthtime;                 // creation time
    
    uint32_t i_size;                      // file bytes count
    uint32_t i_blocks;                    // actual blocks held
    
    struct {
        uint32_t i_db_offset[BEANFS_NDADDR];     // direct addr; offset in data blocks
        uint32_t i_id_offset[BEANFS_NINDIR];     // first is 1st direct and second is 2th indirect
    } i_addr;
    
    uint16_t inode_lock;                  // 0 : unlocked and 1 : locked
};

#define MAX_ENTRY_NAME 20
struct beanfs_dir_entry {
    
    uint32_t d_ino;                        // 0 offset of the inode
    const uint16_t d_enlen;               // 4 entry length
    char d_name[MAX_ENTRY_NAME];                // 5 name_length
    char d_file_type;                           // 25 file type 'd' or 's' or '-'
};                                              // 26 bytes
 
#endif