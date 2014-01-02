//
//  super.c
//  v_filesystem
//
//  Created by WONDER on 12/27/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "beanfs.h"


void create_raw_sb(struct beanfs_super_block *sbp, uint32_t blocks)
{
    sbp->s_blocks_count = blocks;                                   // total disk blocks
    sbp->s_inodes_count = blocks / 10;                              // block number >= 10
    sbp->s_datablocks_count = sbp->s_blocks_count - BOOTBLOCKCOUNT - SUPERBLOCKCOUNT - FREE_SPACEMG_BLOCK_COUNT - sbp->s_inodes_count;
    sbp->s_free_datablocks_count = sbp->s_datablocks_count;
    sbp->s_free_inodes_count = sbp->s_inodes_count;
    sbp->s_free_datablocksmg_block = BOOTBLOCKCOUNT + SUPERBLOCKCOUNT;
    sbp->s_free_inodesmg_block = sbp->s_free_datablocksmg_block + FREE_DATABLOCKMG_BLOCK_COUNT;
    sbp->s_first_inode_block = sbp->s_free_inodesmg_block + FREE_INODESMG_BLOCK_COUNT;
    sbp->s_first_data_block = sbp->s_first_inode_block + sbp->s_inodes_count;
    sbp->s_birthtime = (uint32_t)time(NULL);                            // superblock created time
    sbp->s_mtime = sbp->s_birthtime;
}

int read_superblock(struct beanfs_super_block *sbp, FILE *v_device)
{
    int status = 0;
    if (v_device != NULL) {
        status = read_block(sbp, BOOTBLOCKCOUNT, sizeof(struct beanfs_super_block), 1, v_device);
    } else {
        status = -1;
    }
    return status;
}

int update_superblock(struct beanfs_sb_info *sb_info_p, struct beanfs_super_block *sbp)
{
    int status = 0;
    if (sb_info_p != NULL && sbp != NULL) {
        // update
        sbp->s_blocks_count = sb_info_p->s_blocks_count;
        sbp->s_datablocks_count = sb_info_p->s_datablocks_count;
        sbp->s_free_datablocks_count = sb_info_p->s_free_datablocks_count;
        sbp->s_free_inodes_count = sb_info_p->s_free_inodes_count;
        sbp->s_inodes_count = sb_info_p->s_inodes_count;
        sbp->s_mtime = sb_info_p->s_mtime;
        status = 1;             // updated
    } else {
        status = -1;            // not update
    }
    return status;
}

int sync_superblock(struct beanfs_super_block *sbp, FILE *v_device)
{
    int status = 0;
    if (sbp != NULL) {
        status = write2block(sbp, BOOTBLOCKCOUNT, sizeof(struct beanfs_super_block), 1, v_device);
    } else {
        status = -1;
    }
    return status;
}