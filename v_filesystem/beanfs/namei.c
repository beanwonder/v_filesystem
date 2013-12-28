//
//  namei.c
//  v_filesystem
//
//  Created by WONDER on 12/27/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "beanfs.h"

#define SUPERBLOCKCOUNT 1
#define BOOTBLOCKCOUNT 1
#define FREEBLOCKLISTSCOUNT 1

static void create_root_inode(struct beanfs_inode *root_inode_p)
{
    root_inode_p->i_uid = 0;                           // onwer root
    root_inode_p->i_gid = 0;                           // group root
    root_inode_p->i_file_type = 'd';
    root_inode_p->i_addr.i_db_offset[0] = 0;           // first data block is for root dir
    root_inode_p->i_addr.i_db_offset[1] = 0;
    root_inode_p->i_addr.i_db_offset[2] = 0;
    root_inode_p->i_addr.i_db_offset[3] = 0;
    root_inode_p->i_addr.i_id_offset[0] = 0;
    root_inode_p->i_addr.i_id_offset[1] = 0;
    root_inode_p->i_size = 0;                           // zero byte
    root_inode_p->i_blocks = 1;                         // one data block for initial root
    root_inode_p->i_links = 0;
    root_inode_p->i_mode = 0755;
}

static int init_free_blocks_lists(struct beanfs_super_block *sbp, FILE *v_device)
{
    uint32_t rblocks = 0;
    uint32_t head_block_offset = 1;
    // fill the super_block
    if (sbp->s_data_blocks_count > FREE_BLOCKS_LIST_SIZE) {
        sbp->blocks_list_top = FREE_BLOCKS_LIST_SIZE - 1;
    } else {
        sbp->blocks_list_top = sbp->s_free_blocks_count - 1;            // one data block for root
    }
    // in memory
    for (int i = sbp->blocks_list_top; i >= 0; i--) {
        sbp->s_free_blocks_list_group[i] = head_block_offset + sbp->blocks_list_top - i;
    }
    
    head_block_offset = head_block_offset + sbp->blocks_list_top + 1;
    rblocks = sbp->s_data_blocks_count - sbp->blocks_list_top - 1;
    // write into device
    while (rblocks > 0) {
        if (rblocks >) {
            
        } else {
            
        }
    }
    
    
    
    if (sbp->s_free_inodes_count > FREE_INODES_LIST_SIZE) {
        sbp->inodes_list_top = FREE_INODES_LIST_SIZE;
    } else {
        sbp->inodes_list_top = sbp->s_free_inodes_count - 1;            // one node for root
    }
    
}

static void create_raw_superblock(struct beanfs_super_block *sbp, uint32_t blocks, FILE *v_device)
{
    sbp->s_blocks_count = blocks;                                   // total disk blocks
    sbp->s_inodes_count = blocks / 10;                              // block number >= 10
    sbp->s_data_blocks_count = sbp->s_blocks_count - BOOTBLOCKCOUNT - SUPERBLOCKCOUNT - FREEBLOCKLISTSCOUNT - sbp->s_inodes_count;
    sbp->s_free_inodes_count = sbp->s_inodes_count;
    sbp->s_free_lists_block = BOOTBLOCKCOUNT + SUPERBLOCKCOUNT;
    sbp->s_first_inode_block = sbp->s_free_lists_block + FREEBLOCKLISTSCOUNT;
    sbp->s_first_data_block = sbp->s_first_inode_block + sbp->s_inodes_count;
    sbp->s_ctime = (uint32_t)time(NULL);                            // superblock created time
    sbp->s_mtime = sbp->s_ctime;
}

static int init_beanfs(uint32_t blocks, FILE *virtual_device)
{
    assert(blocks >= 10);
    struct beanfs_super_block sb;
    struct beanfs_inode root_inode;
    // init root entry
    struct beanfs_dir_entry root_dir_entry[2] = {
        {0, 26, ".", 'd'},
        {0, 26, "..", 'd'}
    };
    
    create_root_inode(&root_inode);
    create_raw_superblock(&sb, blocks, virtual_device);
    
    return 0;
}

