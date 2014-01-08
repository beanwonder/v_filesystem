//
//  balloc.c
//  v_filesystem
//
//  Created by WONDER on 1/5/14.
//  Copyright (c) 2014 WONDER. All rights reserved.
//

#include <stdio.h>
#include "beanfs.h"

static uint32_t get_datablock_addr(struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    uint32_t block_addr = UINT32_MAX;
    uint8_t tmptop = -1;
    
    if (sb_info_p->s_free_datablocks_count) {
        sb_info_p->s_free_datablocks_count--;
        tmptop = sb_info_p->s_free_datablocks_group.top--;
        block_addr = sb_info_p->s_free_datablocks_group.list[tmptop];
        // check first datablockgroup
        if (sb_info_p->s_free_datablocks_group.top < 0) {
            // get next group
            read_block(&sb_info_p->s_free_datablocks_group, block_addr, sizeof(struct free_datablocks_group), 1, v_device);
            write2block(&sb_info_p->s_free_datablocks_group, sb_info_p->s_first_data_block, sizeof(struct free_datablocks_group), 1, v_device);
        }
    }
    return block_addr;
}

uint32_t beanfs_alloc_datablock(struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    uint32_t block_addr = UINT32_MAX;
    if (sb_info_p == NULL || v_device == NULL) {
        return block_addr;
    }
    if (sb_info_p->s_free_datablocks_count > 0) {
        // do allocate job
        block_addr = get_datablock_addr(sb_info_p, v_device);
    }
    return block_addr;
}

int beanfs_callback_datablock(struct beanfs_sb_info *sb_info_p, uint32_t block_addr, FILE *v_device)
{
    int status = -1;
    // check
    if (sb_info_p == NULL || v_device == NULL) {
        return status;
    }
    // check block addr
    if (block_addr < UINT32_MAX && block_addr < sb_info_p->s_first_data_block + sb_info_p->s_free_datablocks_count) {
        sb_info_p->s_free_datablocks_count++;
        if (sb_info_p->s_free_datablocks_group.top < FREE_DATABLOCKS_LIST_SIZE) {
            sb_info_p->s_free_datablocks_group.list[++(sb_info_p->s_free_datablocks_group.top)] = block_addr;
        } else {
            write2block(&sb_info_p->s_free_datablocks_group, block_addr, sizeof(struct free_blocks_group), 1, v_device);
            sb_info_p->s_free_datablocks_group.top = 0;
            sb_info_p->s_free_datablocks_group.list[0] = block_addr;
        }
        
        write2block(&sb_info_p->s_free_datablocks_group, sb_info_p->s_free_datablocksmg_block, sizeof(struct free_blocks_group), 1, v_device);
        status = 1;
    }
    return status;                      // if failed return -1 else 1
}