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

// structure of inode on v_disk
struct beanfs_inode {
    
};

// structure of inode in memory
struct beanfs_inode_info {
    
};




#endif
