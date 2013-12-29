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
#include <string.h>
//#include <stdint.h>
#include "beanfs.h"

#define SUPERBLOCKCOUNT 1
#define BOOTBLOCKCOUNT 1
#define FREE_DATABLOCKMG_BLOCK_COUNT 1
#define FREE_INODESMG_BLOCK_COUNT 1
#define FREE_SPACEMG_BLOCK_COUNT 2


static void create_free_blocks_group(struct beanfs_super_block *sbp, struct free_blocks_group *fbg_p, int groups_counts[])
{
    // free_datablocks_group
    if (sbp->s_free_datablocks_count > 0) {
        groups_counts[0] = (sbp->s_free_datablocks_count / FREE_DATABLOCKS_LIST_SIZE) + 1;          // total group including one in freelist block
        fbg_p->data_group.top = (sbp->s_free_datablocks_count % FREE_DATABLOCKS_LIST_SIZE) - 1;   // last group top
        if (fbg_p->data_group.top == 0) {
            fbg_p->data_group.top = FREE_DATABLOCKS_LIST_SIZE - 1;
        }
    } else {
        groups_counts[0] = 0;
        fbg_p->data_group.top = -1;
    }
    
    // free_inodes_group
    if (sbp->s_free_inodes_count > 0) {
        groups_counts[1] = (sbp->s_free_inodes_count / FREE_INODES_LIST_SIZE) + 1;                  // total group including one in freelist block
        fbg_p->inode_group.top = (sbp->s_free_inodes_count % FREE_INODES_LIST_SIZE) - 1;            // last group top
        if (fbg_p->inode_group.top == 0) {
            fbg_p->inode_group.top = FREE_INODES_LIST_SIZE - 1;
        }
    } else {
        groups_counts = 0;
        fbg_p->inode_group.top = -1;
    }
}

static void create_root_dir(struct beanfs_super_block *sbp, struct beanfs_inode *rip, struct beanfs_dir_entry entrys[])
{
    (sbp->s_free_datablocks_count)--;
    (sbp->s_free_inodes_count)--;
    // init inode
    rip->i_mode = 0755;
    rip->i_uid = 0;                           // onwer root
    rip->i_gid = 0;                           // group root
    rip->i_file_type = 'd';
    rip->i_addr.i_db_offset[0] = 0;           // first data block is for root dir
    rip->i_addr.i_db_offset[1] = 0;
    rip->i_addr.i_db_offset[2] = 0;
    rip->i_addr.i_db_offset[3] = 0;
    rip->i_addr.i_id_offset[0] = 0;
    rip->i_addr.i_id_offset[1] = 0;
    rip->i_size = 0;                           // zero byte
    rip->i_blocks = 1;                         // one data block for initial root
    rip->i_links = 0;
    rip->i_birthtime = (uint32_t)time(NULL);
    rip->i_atime = rip->i_birthtime;
    rip->i_ctime = rip->i_birthtime;
    rip->i_mtime = rip->i_birthtime;
    
    // init entrys
    entrys[0].d_file_type = 'd';
    entrys[1].d_file_type = 'd';
    entrys[0].d_ino = 0;
    entrys[1].d_ino = 0;
    entrys[0].d_length = 26;
    entrys[1].d_length = 26;
    strcpy(entrys[0].d_name, ".");
    strcpy(entrys[1].d_name, "..");
}

static void create_raw_sb(struct beanfs_super_block *sbp, uint32_t blocks)
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

static int write2block(const void *buffer, long disk_block, size_t size, size_t count, FILE *vdevice)
{
    int seekresult = 0;
    size_t writeresult = 0;
    int result = 0;
    seekresult = fseek(vdevice, disk_block * BLOCK_SIZE, SEEK_SET);
    writeresult = fwrite(buffer, size, count, vdevice);
    
    if (seekresult == 0 && writeresult > 0) {
        result = 0;                     // correct
    } else {
        result = 1;                     // error
    }
    
    return result;
}

static void write2vdevice(struct beanfs_super_block *sbp, int fbg_counts[],
                          struct free_blocks_group *fbg_p, struct beanfs_inode *rip,
                          struct beanfs_dir_entry r_entrys[], FILE *vdevice)
{
    // write boot
    char bootblock[BLOCK_SIZE] = {0};
    write2block(bootblock, 0, sizeof(char), 1,vdevice);
    // write superblock
    write2block(sbp, BOOTBLOCKCOUNT, sizeof(struct beanfs_super_block), 1, vdevice);

    // write freeblocksgroup
    //struct free_blocks_group tmpblocksgroup;
    struct free_datablocks_group tmp_datablocks_group;
    struct free_inodes_group tmp_inodes_group;
    int first_block = -1;
    int data_group_count = fbg_counts[0];
    int inode_group_count = fbg_counts[1];
    int block_top = -1;

    if (data_group_count > 0) {
        first_block = sbp->s_first_data_block + 1;                             // first block is reserved for rootdir
        if (data_group_count == 1) {
            block_top = fbg_p->data_group.top;
        } else {
            block_top = FREE_DATABLOCKS_LIST_SIZE - 1;
        }
        tmp_datablocks_group.top = block_top;
        for (int i = 0; i < tmp_datablocks_group.top + 1; i++) {
            tmp_datablocks_group.list[tmp_datablocks_group.top - i] = first_block + i;
        }
    } else {
        tmp_datablocks_group.top = -1;
    }
    data_group_count--;
    
    first_block = -1;
    block_top = -1;
    if (inode_group_count > 0) {
        first_block = sbp->s_first_inode_block + 1;                             // first inode is reserved for rootdir
        if (inode_group_count == 1) {
            block_top = fbg_p->inode_group.top;
        } else {
            block_top = FREE_INODES_LIST_SIZE - 1;
        }
        tmp_inodes_group.top = block_top;
        for (int i = 0; i < tmp_inodes_group.top + 1; i++) {
            tmp_inodes_group.list[tmp_inodes_group.top - i] = first_block + i;
        }
    } else {
        tmp_inodes_group.top = -1;
    }
    inode_group_count--;
    
    // write to vdevice
    write2block(&tmp_datablocks_group, sbp->s_free_datablocksmg_block, sizeof(struct free_datablocks_group), 1, vdevice);
    write2block(&tmp_inodes_group, sbp->s_free_inodesmg_block, sizeof(struct free_inodes_group), 1, vdevice);

    //write2block(&tmpblocksgroup, BOOTBLOCKCOUNT+SUPERBLOCKCOUNT, sizeof(struct free_blocks_group), 1, vdevice);
    
    // link free datablock groups
    //struct free_datablocks_group tmp_datablocks_group = tmpblocksgroup.data_group;
    //struct free_inodes_group tmp_inodes_group = tmpblocksgroup.inode_group;
    uint32_t block_dst = 0;
    while (data_group_count >= 0) {
        block_dst = tmp_datablocks_group.list[0];
        for (int i = 0; i < tmp_datablocks_group.top + 1; i++) {
            tmp_datablocks_group.list[i] += FREE_DATABLOCKS_LIST_SIZE;
        }
        write2block(&tmp_datablocks_group, block_dst, sizeof(struct free_datablocks_group), 1, vdevice);
        data_group_count--;
        if (data_group_count == 1) {
            tmp_datablocks_group.top = fbg_p->data_group.top;             // last group elements count
        } else if (data_group_count == 0) {
            tmp_datablocks_group.top = -1;
        }
    }
    block_dst = 0;
    while (inode_group_count >= 0) {
        block_dst = tmp_inodes_group.list[0];
        for (int i = 0; i < tmp_inodes_group.top + 1; i++) {
            tmp_inodes_group.list[i] += FREE_INODES_LIST_SIZE;
        }
        write2block(&tmp_inodes_group, block_dst, sizeof(struct free_inodes_group), 1, vdevice);
        inode_group_count--;
        if (inode_group_count == 1) {
            tmp_inodes_group.top = fbg_p->data_group.top;
        } else if (inode_group_count == 0) {
            tmp_inodes_group.top = -1;
        }
    }
    
    // write root dir
    write2block(rip, sbp->s_first_inode_block, sizeof(struct beanfs_super_block), 1, vdevice);
    write2block(r_entrys, sbp->s_first_data_block, sizeof(struct beanfs_dir_entry), 2, vdevice);
}

int init_beanfs(uint32_t blocks, FILE *virtual_device)
{
    assert(blocks >= 10);
    struct beanfs_super_block sb;
    struct beanfs_inode root_inode;
    struct beanfs_dir_entry root_dir_entrys[2];                 // rootdir entry
    int free_blocks_groups_counts[2] = {0, 0};                  // counts groups respectively [0] for data_block and [1] for inode
    struct free_blocks_group free_blocks_group;
    
    
    create_raw_sb(&sb, blocks);
    create_root_dir(&sb, &root_inode, root_dir_entrys);
    create_free_blocks_group(&sb, &free_blocks_group, free_blocks_groups_counts);
    
    assert(sizeof(struct beanfs_super_block) <= BLOCK_SIZE);
    //assert(sizeof(struct free_blocks_group) <= BLOCK_SIZE);
    assert(sizeof(struct beanfs_inode) <= BLOCK_SIZE);
    assert(2 * sizeof(struct beanfs_dir_entry) <= BLOCK_SIZE);
    write2vdevice(&sb, free_blocks_groups_counts, &free_blocks_group,
                  &root_inode, root_dir_entrys, virtual_device);
    return 0;
}

