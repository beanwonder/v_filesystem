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

static void create_root_dir(struct beanfs_super_block *sbp, struct beanfs_inode *rip, struct beanfs_dir *dir)
{
    (sbp->s_free_datablocks_count)--;
    (sbp->s_free_inodes_count)--;
    // init inode
    rip->i_mode = 0755;
    rip->i_uid = 0;                           // onwer root
    rip->i_gid = 0;                           // group root
    rip->i_addr.d_addr[0] = sbp->s_first_data_block;           // first data block is for root dir
    rip->i_addr.d_addr[1] = 0;
    rip->i_addr.d_addr[2] = 0;
    rip->i_addr.d_addr[3] = 0;
    rip->i_addr.id_addr[0] = 0;
    rip->i_addr.id_addr[1] = 0;
    rip->i_size = 0;                           // zero byte
    rip->i_blocks = 1;                         // one data block for initial root
    rip->i_links = 0;
    rip->i_birthtime = (uint32_t)time(NULL);
    rip->i_atime = rip->i_birthtime;
    rip->i_ctime = rip->i_birthtime;
    rip->i_mtime = rip->i_birthtime;
    
    // init dot and dotdot
    dir->len = 2;
    dir->entrys[0].d_file_type = 'd';
    dir->entrys[1].d_file_type = 'd';
    dir->entrys[0].d_ino = 0;
    dir->entrys[1].d_ino = 0;
    strcpy(dir->entrys[0].d_name, ".");
    strcpy(dir->entrys[1].d_name, "..");
}

int write2block(const void *buffer, long dst_block, size_t size, size_t count, FILE *vdevice)
{
    int seekresult = 0;
    int writeresult = 0;
    int status = 0;
    seekresult = fseek(vdevice, dst_block * BLOCK_SIZE, SEEK_SET);
    writeresult = (int)fwrite(buffer, size, count, vdevice);
    
    if (seekresult == 0 && writeresult > 0) {
        status = writeresult;                     // write successed
    } else {
        status = -1;                              // failed
    }
    
    return status;
}

static void write2vdevice(struct beanfs_super_block *sbp, int fbg_counts[],
                          struct free_blocks_group *fbg_p, struct beanfs_inode *rip,
                          struct beanfs_dir *dir, FILE *vdevice)
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
    write2block(dir, sbp->s_first_data_block, sizeof(struct beanfs_dir), 1,vdevice);
}

int init_beanfs(uint32_t blocks, FILE *virtual_device)
{
    assert(blocks >= 10);
    struct beanfs_super_block sb;
    struct beanfs_inode root_inode;
    struct beanfs_dir root_dir;                                 // rootdir entry
    int free_blocks_groups_counts[2] = {0, 0};                  // counts groups respectively [0] for data_block and [1] for inode
    struct free_blocks_group free_blocks_group;
    
    
    create_raw_sb(&sb, blocks);
    create_root_dir(&sb, &root_inode, &root_dir);
    create_free_blocks_group(&sb, &free_blocks_group, free_blocks_groups_counts);
    
    assert(sizeof(struct beanfs_super_block) <= BLOCK_SIZE);
    assert(sizeof(struct beanfs_inode) <= BLOCK_SIZE);
    assert(sizeof(struct beanfs_dir) <= BLOCK_SIZE);
    write2vdevice(&sb, free_blocks_groups_counts, &free_blocks_group,
                  &root_inode, &root_dir, virtual_device);
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
    if (block_addr >= sb_info_p->s_first_data_block && block_addr <= sb_info_p->s_first_data_block + sb_info_p->s_datablocks_count - 1) {
        // read data block
        status = read_block(buffer, block_addr, sizeof(char), BLOCK_SIZE, v_device);
    } else {
        status = -1;
    }
    return status;                              // return the number of byte read
}