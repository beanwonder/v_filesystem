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

#define BLOCK_SIZE 512
// first inode is for root
#define BEANFS_ROOT_INODE_OFFSET
#define FREE_BLOCKS_LIST_SIZE 60
#define FREE_INODES_LIST_SIZE 60

// structure of the superblock on virtual
// using logical block address from 0 to user defined number
struct beanfs_super_block {
    
    unsigned long s_blocks_count;
    unsigned long s_inodes_count;
    unsigned long s_data_blocks_count;
    unsigned long s_free_blocks_count;
    unsigned long s_free_inodes_count;
    
    unsigned long s_free_blocks_list_group[FREE_BLOCKS_LIST_SIZE];
    unsigned long s_free_inodes_list_group[FREE_INODES_LIST_SIZE];
    unsigned short blocks_list_top;              // number of blocks in free block list
    unsigned short inodes_list_top;              // number of inodes in free inode list
    
    unsigned long first_data_block;
    unsigned long first_inode;
    
    //unsigned short block_list_lock;             //0 stand for unlock and 1 stand for unlock
    //unsigned short inode_list_lock;
    
    unsigned long modify_time;                  // modified time stamp
};

// structure of superblock in memomry
struct beanfs_sb_info {
    
    char i_ftype;
    unsigned long s_blocks_count;
    unsigned long s_inodes_count;
    unsigned long s_free_blocks_count;
    unsigned long s_free_inodes_count;
    
    unsigned long s_free_blocks_list_group[60]; // array of blocks offset
    unsigned long s_free_inodes_list_group[60]; // array of inodes offset
    unsigned short blocks_list_top;              // number of blocks in free block list
    unsigned short inodes_list_top;              // number of inodes in free inode list
    
    unsigned long first_data_block;             // first data block
    unsigned long first_inode;                  // first inode
    
    unsigned short blocks_list_lock;             //0 stand for unlock and 1 stand for unlock
    unsigned short inodes_list_lock;
    
    unsigned short is_modified;                 // flag for wheather modified
    unsigned long modify_time;                  // modified time stamp
};


#define BEANFS_NDADDR 4
#define BEANFS_NINDIR 2

// structure of inode on v_disk
struct beanfs_inode {
    
    char i_file_type;
    unsigned short i_mode;                     // permission
    unsigned short i_links;                    // file link count
    unsigned long i_uid;                       // file onwer
    unsigned long i_gid;                       // file owner group
    
    unsigned long i_atime;                     // last access time
    unsigned long i_mtime;                     // last modify time
    unsigned long i_ctime;                     // last inode changetime
    unsigned long i_birthtime;                 // creation time
    
    unsigned long i_size;                      // file bytes count
    unsigned long i_blocks;                    // actual blocks held
    
    struct {
        unsigned long i_db_offset[BEANFS_NDADDR];     // direct addr; number in data_blocks
        unsigned long i_id_offset[BEANFS_NINDIR];     // first is 1st direct and second is 2th indirect
    } i_addr;
    
};

// structure of inode in memory
struct beanfs_inode_info {
    
    unsigned short i_access_count;             // access count
    
    unsigned short i_mode;                     // permission
    unsigned short i_links;                    // file link count
    unsigned long i_uid;                       // file onwer
    unsigned long i_gid;                       // file owner group
    
    unsigned long i_atime;                     // last access time
    unsigned long i_mtime;                     // last modify time
    unsigned long i_ctime;                     // last inode changetime
    unsigned long i_birthtime;                 // creation time
    
    unsigned long i_size;                      // file bytes count
    unsigned long i_blocks;                    // actual blocks held
    
    struct {
        unsigned long i_db_offset[BEANFS_NDADDR];     // direct addr; offset in data blocks
        unsigned long i_id_offset[BEANFS_NINDIR];     // first is 1st direct and second is 2th indirect
    } i_addr;
    
    unsigned short inode_lock;                  // 0 : unlocked and 1 : locked
};

#define MAX_ENTRY_NAME 20
struct beanfs_dir_entry {
    
    unsigned long d_ino;                        // 0 offset of the inode
    const unsigned short d_enlen;               // 4 entry length
    char d_name[MAX_ENTRY_NAME];                // 5 name_length
    char d_file_type;                           // 25 file type 'd' or 's' or '-'
};                                              // 26 bytes

/**********************************************************************************
 
 

#endif