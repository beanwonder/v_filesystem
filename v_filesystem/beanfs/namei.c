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

#define SUPERBLOCKNUM 1
#define BOOTBLOCKNUM 1


static void init_root_inode(struct beanfs_inode *root_inode_p)
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

static void init_free_lists_top(struct beanfs_super_block *sbp)
{
    if (sbp->s_data_blocks_count > FREE_BLOCKS_LIST_SIZE) {
        sbp->blocks_list_top = FREE_BLOCKS_LIST_SIZE - 1;
    } else {
        sbp->blocks_list_top = sbp->s_free_blocks_count - 1;            // one data block for root
    }
    
    if (sbp->s_free_inodes_count > FREE_INODES_LIST_SIZE) {
        sbp->inodes_list_top = FREE_INODES_LIST_SIZE;
    } else {
        sbp->inodes_list_top = sbp->s_free_inodes_count - 1;            // one node for root
    }
    
    
}

static void init_superblock(struct beanfs_super_block *sbp, unsigned long blocks)
{
    sbp->s_blocks_count = blocks;                                   // disk blocks
    sbp->s_inodes_count = blocks / 10;                              // block number >= 10
    sbp->s_data_blocks_count = sbp->s_blocks_count - 1;
    
    sbp->s_free_inodes_count = sbp->s_inodes_count - 1;             // one inode for root dir
    sbp->first_inode = BOOTBLOCKNUM + SUPERBLOCKNUM - 1;
    sbp->first_data_block = BOOTBLOCKNUM + SUPERBLOCKNUM + sbp->s_inodes_count - 1;
    sbp->modify_time = (unsigned long)time(NULL);
    
    init_free_lists_top(sbp);
}



static int init_beanfs(unsigned long blocks, FILE virtual_device)
{
    assert(blocks > 10);
    struct beanfs_super_block sb;
    struct beanfs_inode root_inode;
    
    init_superblock(&sb, blocks);
    init_root_inode(&root_inode);
    // init root entry
    struct beanfs_dir_entry root_dir_entry[2] = {
        {0, 26, ".", 'd'},
        {0, 26, "..", 'd'}
    };
    // init grouping lists
    //init_free_blocks_list(&sb);
    //init_free_inodes_list(&sb);
    
    
    
    return 0;
}