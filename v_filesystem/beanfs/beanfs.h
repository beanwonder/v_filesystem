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

// structure of the superblock on virtual
// using logical block address from 0 to user defined number
struct beanfs_super_block {
    
    unsigned long s_blocks_count;
    unsigned long s_inodes_count;
    unsigned long s_free_blocks_count;
    unsigned long s_free_inodes_count;
    
    unsigned long s_free_blocks_list_group[60];
    unsigned long s_free_inodes_list_group[60];
    unsigned short block_list_top;              // number of blocks in free block list
    unsigned short inode_list_top;              // number of inodes in free inode list
    
    unsigned long first_data_block;
    unsigned long first_inode;
    
    unsigned short block_list_lock;             //0 stand for unlock and 1 stand for unlock
    unsigned short inode_list_lock;
    
    unsigned long modify_time;                  // modified time stamp
};

// structure of superblock in memomry
struct beanfs_sb_info {
    
    unsigned long s_blocks_count;
    unsigned long s_inodes_count;
    unsigned long s_free_blocks_count;
    unsigned long s_free_inodes_count;
    
    unsigned long s_free_blocks_list_group[60];
    unsigned long s_free_inodes_list_group[60];
    unsigned short block_list_top;              // number of blocks in free block list
    unsigned short inode_list_top;              // number of inodes in free inode list
    
    unsigned long first_data_block;             // first data block
    unsigned long first_inode;                  // first inode
    
    unsigned short block_list_lock;             //0 stand for unlock and 1 stand for unlock
    unsigned short inode_list_lock;
    
    unsigned short is_modified;                 // flag for wheather modified
    unsigned long modify_time;                  // modified time stamp
};

#define BEANFS_NDADDR 4
#define BEANFS_NINDIR 2
// structure of inode on v_disk
struct beanfs_inode {
    
    char bi_ftype;
    unsigned short i_mode;                     // permission
    unsigned short i_nlink;                    // file link count
    unsigned long i_uid;                       // file onwer
    unsigned long i_gid;                       // file owner group
    
    unsigned long i_atime;                     // last access time
    unsigned long i_mtime;                     // last modify time
    unsigned long i_ctime;                     // last inode changetime
    unsigned long i_birthtime;                 // creation time
    
    unsigned long i_size;                      // file bytes count
    unsigned long i_blocks;                    // actual blocks held
    
    struct {
        unsigned long i_db[BEANFS_NDADDR];     // direct addr
        unsigned long i_id[BEANFS_NINDIR];     // first is 1st direct and second is 2th indirect
    } bi_addr;
    
};

// structure of inode in memory
struct beanfs_inode_info {
    
    unsigned long i_num;                       // inode number in memory
    unsigned short i_access_count;             // access count
    
    char i_ftype;
    unsigned short i_mode;                     // permission
    unsigned short i_nlink;                    // file link count
    unsigned long i_uid;                       // file onwer
    unsigned long i_gid;                       // file owner group
    
    unsigned long i_atime;                     // last access time
    unsigned long i_mtime;                     // last modify time
    unsigned long i_ctime;                     // last inode changetime
    unsigned long i_birthtime;                 // creation time
    
    unsigned long i_size;                      // file bytes count
    unsigned long i_blocks;                    // actual blocks held
    
    struct {
        unsigned long i_db[BEANFS_NDADDR];     // direct addr
        unsigned long i_id[BEANFS_NINDIR];     // first is 1st direct and second is 2th indirect
    } bi_addr;
    
    unsigned short inode_lock;                 // 0 : unlocked and 1 : locked
};




#endif
