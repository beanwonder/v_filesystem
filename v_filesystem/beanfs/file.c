//
//  file.c
//  v_filesystem
//
//  Created by WONDER on 12/27/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//
#include <assert.h>
#include <string.h>
#include "beanfs.h"

int beanfs_fread(struct beanfs_sb_info *sb_info_p, struct beanfs_file *beanfs_file,
                 char buffer[BLOCK_SIZE], uint32_t block_no, FILE *v_device)
{
    int status = -1;
    uint32_t index_block[512 / sizeof(uint32_t)];
    
    if (block_no < beanfs_file->inode_info.i_blocks) {
        if (block_no < 4) {
            // direct access
            status = read_data_block(sb_info_p, buffer, beanfs_file->inode_info.i_addr.d_addr[block_no], v_device);
        } else if (block_no < 4 + BLOCK_SIZE) {
            // once index addressing
            status = read_data_block(sb_info_p, index_block, beanfs_file->inode_info.i_addr.id_addr[0], v_device);
            status = read_data_block(sb_info_p, buffer, index_block[block_no - 4], v_device);
        } else {
            // twice index addressing
            status = read_data_block(sb_info_p, index_block, beanfs_file->inode_info.i_addr.id_addr[1], v_device);
            status = read_data_block(sb_info_p, index_block, index_block[(block_no - 4 - BLOCK_SIZE) / 128], v_device);
            status = read_data_block(sb_info_p, index_block, buffer[(block_no - 4 - BLOCK_SIZE) % 128], v_device);
        }
    } else {
        fprintf(stderr, "error out of the file range \n");
    }
    return status;
}