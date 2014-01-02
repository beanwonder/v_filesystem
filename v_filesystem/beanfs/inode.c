//
//  inode.c
//  v_filesystem
//
//  Created by WONDER on 1/2/14.
//  Copyright (c) 2014 WONDER. All rights reserved.
//

#include <stdio.h>
#include "beanfs.h"


// beanfs_inode to beanfs_inode_info
int beanfs_transform2inode_info(struct beanfs_inode *inode_p, struct beanfs_inode_info *inode_info_p)
{
    int status = 0;
    // check pointer
    if (inode_p != NULL && inode_info_p != NULL) {
        for (int i = 0; i < 4; i++) {
            inode_info_p->i_addr.d_addr[i] = inode_p->i_addr.d_addr[i];
        }
        for (int i = 0; i < 2; i++) {
            inode_info_p->i_addr.id_addr[i] = inode_p->i_addr.id_addr[i];
        }
        inode_info_p->i_atime = inode_p->i_atime;
        inode_info_p->i_birthtime = inode_p->i_birthtime;
        inode_info_p->i_ctime = inode_p->i_ctime;
        inode_info_p->i_mtime = inode_p->i_mtime;
        inode_info_p->i_blocks = inode_p->i_blocks;
        inode_info_p->i_size = inode_p->i_size;
        inode_info_p->i_gid = inode_p->i_gid;
        inode_info_p->i_uid = inode_p->i_uid;
        inode_info_p->i_mode = inode_p->i_mode;
        inode_info_p->i_links = inode_p->i_links;
    } else {
        status = -1;
    }
    return status;
}

int beanfs_transform2inode(struct beanfs_inode_info *inode_info_p, struct beanfs_inode *inode_p)
{
    int status = 0;
    // check pointer
    if (inode_p != NULL && inode_info_p != NULL) {
        for (int i = 0; i < 4; i++) {
            inode_p->i_addr.d_addr[i] = inode_info_p->i_addr.d_addr[i];
        }
        for (int i = 0; i < 2; i++) {
            inode_p->i_addr.id_addr[i] = inode_info_p->i_addr.id_addr[i];
        }
        inode_p->i_atime = inode_info_p->i_atime;
        inode_p->i_birthtime = inode_info_p->i_birthtime;
        inode_p->i_ctime = inode_info_p->i_ctime;
        inode_p->i_mtime = inode_info_p->i_mtime;
        inode_p->i_blocks = inode_info_p->i_blocks;
        inode_p->i_gid = inode_info_p->i_gid;
        inode_p->i_uid = inode_info_p->i_uid;
        inode_p->i_mode = inode_info_p->i_mode;
        inode_p->i_links = inode_info_p->i_links;
    } else {
        status = -1;
    }
    return status;

}

int beanfs_write_inode(struct beanfs_sb_info *sb_info_p, struct beanfs_inode *inode_p, uint32_t dst_addr, FILE *v_device)
{
    int status = 0;
    // check inode address
    if (dst_addr >= sb_info_p->s_first_inode_block && dst_addr <= sb_info_p->s_first_inode_block + sb_info_p->s_inodes_count - 1) {
        status = write2block(inode_p, dst_addr, sizeof(struct beanfs_inode), 1, v_device);
    } else {
        status = -1;
    }
    
    return status;
}

int beanfs_read_inode(struct beanfs_sb_info *sb_info_p, struct beanfs_inode *inode_p, uint32_t inode_addr, FILE *v_device)
{
    int status = 0;
    // check inode address
    if (inode_addr >= sb_info_p->s_first_inode_block && inode_addr <= sb_info_p->s_first_inode_block + sb_info_p->s_inodes_count - 1) {
        status = read_block(inode_p, inode_addr, sizeof(struct beanfs_inode), 1, v_device);
    } else {
        status = -1;
    }
    return status;
}

