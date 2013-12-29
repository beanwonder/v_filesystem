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
#include "beanfs.h"

#define SUPERBLOCKCOUNT 1
#define BOOTBLOCKCOUNT 1
#define FREEBLOCKLISTSCOUNT 1


static void create_free_blocks_lists(struct beanfs_super_block *sbp, struct free_blocks_lists *fbl_p, int groups_counts[])
{
    // free_datablocks_lists
    if (sbp->s_free_datablocks_count > 0) {
        groups_counts[0] = (sbp->s_free_datablocks_count / FREE_DATABLOCKS_LIST_SIZE) + 1;          // total group including one in freelist block
        fbl_p->free_datablocks_top = (sbp->s_free_datablocks_count % FREE_DATABLOCKS_LIST_SIZE) - 1;// last group top
    } else {
        groups_counts[0] = 0;
        fbl_p->free_datablocks_top = -1;
    }
    
    // free_inodes_lists
    if (sbp->s_free_inodes_count > 0) {
        groups_counts[1] = (sbp->s_free_inodes_count / FREE_INODES_LIST_SIZE) + 1;                  // total group including one in freelist block
        fbl_p->free_inodes_top = (sbp->s_free_inodes_count % FREE_INODES_LIST_SIZE) - 1;            // last group top
    } else {
        groups_counts = 0;
        fbl_p->free_inodes_top = -1;
    }
}

static void create_root_dir(struct beanfs_super_block *sbp, struct beanfs_inode *rip, struct beanfs_dir_entry entrys[])
{
    (sbp->s_free_datablocks_count)--;
    (sbp->s_free_inodes_count)--;
    // init inode
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
    rip->i_mode = 0755;
    
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
    sbp->s_datablocks_count = sbp->s_blocks_count - BOOTBLOCKCOUNT - SUPERBLOCKCOUNT - FREEBLOCKLISTSCOUNT - sbp->s_inodes_count;
    sbp->s_free_datablocks_count = sbp->s_datablocks_count;
    sbp->s_free_inodes_count = sbp->s_inodes_count;
    sbp->s_free_lists_block = BOOTBLOCKCOUNT + SUPERBLOCKCOUNT;
    sbp->s_first_inode_block = sbp->s_free_lists_block + FREEBLOCKLISTSCOUNT;
    sbp->s_first_data_block = sbp->s_first_inode_block + sbp->s_inodes_count;
    sbp->s_ctime = (uint32_t)time(NULL);                            // superblock created time
    sbp->s_mtime = sbp->s_ctime;
}

static void write2vdevice(struct beanfs_super_block *sbp, struct free_blocks_lists *fbl_p,
                          struct beanfs_inode *rip, struct beanfs_dir_entry r_entrys[], FILE *vdevice)
{
    // write boot
    fseek(vdevice, 0, SEEK_SET);
    fwrite(0, sizeof(char), BLOCK_SIZE, vdevice);
    // write superblock
    fseek(vdevice, BOOTBLOCKCOUNT * BLOCK_SIZE, SEEK_SET);
    fwrite(sbp, sizeof(struct beanfs_super_block), SUPERBLOCKCOUNT, vdevice);
    // write freeblockslists
    struct free_blocks_lists tempblocklists;
    
}

static int init_beanfs(uint32_t blocks, FILE *virtual_device)
{
    assert(blocks >= 10);
    struct beanfs_super_block sb;
    struct beanfs_inode root_inode;
    struct beanfs_dir_entry root_dir_entrys[2];                 // rootdir entry
    int free_block_lists_groups_counts[2] = {0, 0};             // counts groups respectively [0] for data_block and [1] for inode
    struct free_blocks_lists free_blocks_lists;
    
    create_raw_sb(&sb, blocks);
    create_root_dir(&sb, &root_inode, root_dir_entrys);
    create_free_blocks_lists(&sb, &free_blocks_lists, free_block_lists_groups_counts);
    
    assert(sizeof(struct beanfs_super_block) <= BLOCK_SIZE);
    assert(sizeof(struct free_blocks_lists) <= BLOCK_SIZE);
    assert(sizeof(struct beanfs_inode) <= BLOCK_SIZE);
    assert(sizeof(struct beanfs_dir_entry) <= BLOCK_SIZE);
    write2vdevice(&sb, &free_blocks_lists, &root_inode, root_dir_entrys, virtual_device);
    return 0;
}

