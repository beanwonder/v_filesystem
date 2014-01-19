//
//  namei.c
//  v_filesystem
//
//  Created by WONDER on 12/27/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#include <assert.h>
#include <time.h>
#include <string.h>
#include "beanfs.h"


static void init_free_group(struct beanfs_super_block *sbp, FILE *v_device)
{
    struct free_inodes_group tmp_inode_group = {-1, {0}};
    struct free_datablocks_group tmp_datablock_group = {-1, {0}};
    uint32_t free_block_cnt = 0;
    uint32_t tmp_block_addr = UINT32_MAX;
    
    // free_inodes_group
    free_block_cnt = sbp->s_free_inodes_count;
    if (free_block_cnt > FREE_INODES_LIST_SIZE) {
        tmp_inode_group.top = FREE_INODES_LIST_SIZE - 1;
        free_block_cnt = free_block_cnt - FREE_INODES_LIST_SIZE;
    } else {
        tmp_inode_group.top = free_block_cnt - 1;
        free_block_cnt = 0;
    }
    for (int8_t i = 0; i <= tmp_inode_group.top; i++) {
        tmp_inode_group.list[i] = sbp->s_first_inode_block + sbp->s_inodes_count - sbp->s_free_inodes_count + i;
    }
    write2block(&tmp_inode_group, sbp->s_free_inodesmg_block, sizeof(struct free_inodes_group), 1, v_device);
    while (free_block_cnt > 0) {
        tmp_block_addr = tmp_inode_group.list[0];
        if (free_block_cnt > FREE_INODES_LIST_SIZE) {
            tmp_inode_group.top = FREE_INODES_LIST_SIZE - 1;
            free_block_cnt = free_block_cnt - FREE_INODES_LIST_SIZE;
        } else {
            tmp_inode_group.top = free_block_cnt - 1;
            free_block_cnt = 0;
        }
        for (int8_t i = 0; i <= tmp_inode_group.top; i++) {
            tmp_inode_group.list[i] = tmp_inode_group.list[i] + FREE_INODES_LIST_SIZE;
        }
        write2block(&tmp_inode_group, tmp_block_addr, sizeof(struct free_inodes_group), 1, v_device);
    }
    tmp_block_addr = tmp_inode_group.list[0];
    tmp_inode_group.top = -1;
    write2block(&tmp_inode_group, tmp_block_addr, sizeof(struct free_inodes_group), 1, v_device);
    
    // free data block
    free_block_cnt = sbp->s_free_datablocks_count;
    if (free_block_cnt > FREE_DATABLOCKS_LIST_SIZE) {
        tmp_datablock_group.top = FREE_DATABLOCKS_LIST_SIZE - 1;
        free_block_cnt = free_block_cnt - FREE_DATABLOCKS_LIST_SIZE;
    } else {
        tmp_datablock_group.top = free_block_cnt - 1;
        free_block_cnt = 0;
    }
    for (int8_t i = 0; i <= tmp_datablock_group.top; i++) {
        tmp_datablock_group.list[i] = sbp->s_first_data_block + sbp->s_datablocks_count - sbp->s_free_datablocks_count + i;
    }
    write2block(&tmp_datablock_group, sbp->s_free_datablocksmg_block, sizeof(struct free_datablocks_group), 1, v_device);
    while (free_block_cnt > 0) {
        tmp_block_addr = tmp_datablock_group.list[0];
        if (free_block_cnt > FREE_DATABLOCKS_LIST_SIZE) {
            tmp_datablock_group.top = FREE_DATABLOCKS_LIST_SIZE - 1;
            free_block_cnt = free_block_cnt - FREE_DATABLOCKS_LIST_SIZE;
        } else {
            tmp_datablock_group.top = free_block_cnt - 1;
            free_block_cnt = 0;
        }
        for (int8_t i = 0; i <= tmp_datablock_group.top; i++) {
            tmp_datablock_group.list[i] = tmp_datablock_group.list[i] + FREE_DATABLOCKS_LIST_SIZE;
        }
        write2block(&tmp_datablock_group, tmp_block_addr, sizeof(struct free_datablocks_group), 1, v_device);
    }
    tmp_block_addr = tmp_datablock_group.list[0];
    tmp_datablock_group.top = -1;
    write2block(&tmp_datablock_group, tmp_block_addr, sizeof(struct free_datablocks_group), 1, v_device);
}

static void init_root_dir(struct beanfs_super_block *sbp, FILE *v_device)
{
    struct beanfs_inode rinode;
    struct beanfs_dir root_dir;
    
    (sbp->s_free_datablocks_count)--;
    (sbp->s_free_inodes_count)--;
    // init inode
    rinode.i_mode = 0755;
    rinode.i_uid = 0;                           // onwer root
    rinode.i_gid = 0;                           // group root
    rinode.i_addr.d_addr[0] = sbp->s_first_data_block;           // first data block is for root dir
    rinode.i_addr.d_addr[1] = 0;
    rinode.i_addr.d_addr[2] = 0;
    rinode.i_addr.d_addr[3] = 0;
    rinode.i_addr.id_addr[0] = 0;
    rinode.i_addr.id_addr[1] = 0;
    rinode.i_size = 0;                           // zero byte
    rinode.i_blocks = 1;                         // one data block for initial root
    rinode.i_links = 0;
    rinode.i_birthtime = (uint32_t)time(NULL);
    rinode.i_atime = rinode.i_birthtime;
    rinode.i_ctime = rinode.i_birthtime;
    rinode.i_mtime = rinode.i_birthtime;
    
    // init dot and dotdot
    root_dir.len = 2;
    root_dir.entrys[0].d_file_type = 'd';
    root_dir.entrys[1].d_file_type = 'd';
    root_dir.entrys[0].d_ino = 0;
    root_dir.entrys[1].d_ino = 0;
    strcpy(root_dir.entrys[0].d_name, ".");
    strcpy(root_dir.entrys[1].d_name, "..");
    
    write2block(&rinode, sbp->s_first_inode_block, sizeof(struct beanfs_inode), 1, v_device);
    write2block(&root_dir, sbp->s_first_data_block, sizeof(struct beanfs_dir), 1, v_device);
}

int write2block(const void *buffer, long dst_block, size_t size, size_t count, FILE *v_device)
{
    int seekresult = 0;
    int writeresult = 0;
    int status = 0;
    seekresult = fseek(v_device, dst_block * BLOCK_SIZE, SEEK_SET);
    writeresult = (int)fwrite(buffer, size, count, v_device);
    
    if (seekresult == 0 && writeresult > 0) {
        status = writeresult;                     // write successed
    } else {
        status = -1;                              // failed
    }
    
    return status;
}

int init_beanfs(uint32_t blocks, FILE *virtual_device)
{
    assert(blocks >= 10);
    struct beanfs_super_block sb;
    // write boot
    char bootblock[BLOCK_SIZE] = {0};
    write2block(bootblock, 0, sizeof(char), BLOCK_SIZE, virtual_device);
    
    create_raw_sb(&sb, blocks, virtual_device);
    //create_root_dir(&sb, &root_inode, &root_dir);
    init_root_dir(&sb, virtual_device);
    init_free_group(&sb, virtual_device);
    write2block(&sb, BOOTBLOCKCOUNT, sizeof(struct beanfs_super_block), 1, virtual_device);
    return 0;
}
// ----------------------------------------------------------------------

int read_block(void *buffer, long dst_block, size_t size , size_t count, FILE *v_device)
{
    int seekresult = 0;
    int readresult = 0;
    int status = 0;
    seekresult = fseek(v_device, dst_block * BLOCK_SIZE, SEEK_SET);
    readresult = (int)fread(buffer, size, count, v_device);
    
    if (seekresult == 0 && readresult > 0) {
        status = readresult;                // read successed
    } else {
        status = -1;                        // failed
    }
    return status;
}

int read_data_block(struct beanfs_sb_info *sb_info_p, void *buffer, uint32_t block_addr, FILE *v_device)
{
    int status = 0;
    // check wheather block addr out of data block index
    if (block_addr >= sb_info_p->s_first_data_block &&
        block_addr <= sb_info_p->s_first_data_block + sb_info_p->s_datablocks_count - 1) {
        // read data block
        status = read_block(buffer, block_addr, sizeof(char), BLOCK_SIZE, v_device);
    } else {
        status = -1;
    }
    return status;                              // return the number of byte read
}