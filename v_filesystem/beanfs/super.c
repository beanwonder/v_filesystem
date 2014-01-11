//
//  super.c
//  v_filesystem
//
//  Created by WONDER on 12/27/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#include <stdlib.h>
#include <time.h>
#include "beanfs.h"

int beanfs_transform2sb_info(struct beanfs_super_block *sbp, struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    int status = -1;
    struct free_blocks_group free_block_group;
    
    if (sbp != NULL && sb_info_p != NULL) {
        // copy to sb_info_p
        sb_info_p->s_birthtime = sbp->s_birthtime;
        sb_info_p->s_blocks_count = sbp->s_blocks_count;
        sb_info_p->s_inodes_count = sbp->s_inodes_count;
        sb_info_p->s_datablocks_count = sbp->s_datablocks_count;
        sb_info_p->s_first_data_block = sbp->s_first_data_block;
        sb_info_p->s_first_inode_block = sbp->s_first_inode_block;
        sb_info_p->s_free_datablocks_count = sbp->s_free_datablocks_count;
        sb_info_p->s_free_datablocksmg_block = sbp->s_free_datablocksmg_block;
        sb_info_p->s_free_inodes_count = sbp->s_free_inodes_count;
        sb_info_p->s_free_inodesmg_block = sbp->s_free_inodesmg_block;
        sb_info_p->s_mtime = sbp->s_mtime;
        
        // read free data gourp and inode group
        read_block(&free_block_group.inode_group, sb_info_p->s_free_inodesmg_block, sizeof(struct free_inodes_group), 1, v_device);
        read_block(&free_block_group.data_group, sb_info_p->s_free_datablocksmg_block, sizeof(struct free_datablocks_group), 1, v_device);
        sb_info_p->s_free_inodes_group = free_block_group.inode_group;
        sb_info_p->s_free_datablocks_group = free_block_group.data_group;
    }
    return status;
}

void create_raw_sb(struct beanfs_super_block *sbp, uint32_t blocks)
{
    sbp->s_blocks_count = blocks;                                   // total disk blocks
    sbp->s_inodes_count = blocks / 10;                              // block number >= 10
    sbp->s_datablocks_count = sbp->s_blocks_count - BOOTBLOCKCOUNT - SUPERBLOCKCOUNT - FREE_SPACEMG_BLOCK_COUNT - sbp->s_inodes_count;
    sbp->s_free_datablocks_count = sbp->s_datablocks_count;
    sbp->s_free_inodes_count = sbp->s_inodes_count;
    sbp->s_free_inodesmg_block = BOOTBLOCKCOUNT + SUPERBLOCKCOUNT;
    sbp->s_free_datablocksmg_block = sbp->s_free_inodesmg_block + FREE_INODESMG_BLOCK_COUNT;
    sbp->s_first_inode_block = sbp->s_free_inodesmg_block + FREE_DATABLOCKMG_BLOCK_COUNT;
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

int update_superblock(struct beanfs_sb_info *sb_info_p, struct beanfs_super_block *sbp, FILE *v_device)
{
    int status = 0;
    if (sb_info_p != NULL && sbp != NULL) {
        // update
        sbp->s_free_datablocks_count = sb_info_p->s_free_datablocks_count;
        sbp->s_free_inodes_count = sb_info_p->s_free_inodes_count;
        sbp->s_mtime = sb_info_p->s_mtime;
        status = sync_superblock(sbp, v_device);
    } else {
        status = -1;            // not update
    }
    return status;
}
