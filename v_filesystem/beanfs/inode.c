//
//  inode.c
//  v_filesystem
//
//  Created by WONDER on 1/2/14.
//  Copyright (c) 2014 WONDER. All rights reserved.
//

#include <time.h>
#include "beanfs.h"

int beanfs_read_inode(struct beanfs_sb_info *sb_info_p, struct beanfs_inode *inode_p,
                      uint32_t inode_addr, FILE *v_device)
{
    int status = 0;
    // check inode address
    if (inode_addr >= sb_info_p->s_first_inode_block
        && inode_addr <= sb_info_p->s_first_inode_block + sb_info_p->s_inodes_count - 1) {
        
        status = read_block(inode_p, inode_addr, sizeof(struct beanfs_inode), 1, v_device);
    } else {
        status = -1;
    }
    return status;
}

int beanfs_write_inode(struct beanfs_sb_info *sb_info_p, struct beanfs_inode *inode_p,
                       uint32_t dst_addr, FILE *v_device)
{
    int status = 0;
    // check inode address
    if (dst_addr >= sb_info_p->s_first_inode_block
        && dst_addr <= sb_info_p->s_first_inode_block + sb_info_p->s_inodes_count - 1) {
        
        status = write2block(inode_p, dst_addr, sizeof(struct beanfs_inode), 1, v_device);
    } else {
        status = -1;
    }
    
    return status;
}

static int init_inode_info(struct beanfs_inode_info *inode_info_p, int ino)
{
    int status = -1;    // -1 stand for failed when init
    int curtime = (int)time(NULL);
    if (inode_info_p != NULL) {
        inode_info_p->i_ino = ino;
        inode_info_p->i_birthtime = curtime;
        inode_info_p->i_atime = curtime;
        inode_info_p->i_ctime = curtime;
        inode_info_p->i_mtime = curtime;
    }
    return status;
}

static uint32_t beanfs_get_inode(struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    uint32_t ino = UINT32_MAX;
    uint32_t inode_addr = 0;
    int tmptop = -1;
    
    if (sb_info_p->s_free_inodes_count) {
        // still have free node
        sb_info_p->s_free_inodes_count--;
        tmptop = sb_info_p->s_free_inodes_group.top--;
        inode_addr = sb_info_p->s_free_inodes_group.list[tmptop];
        // check weather top == 0
        if (sb_info_p->s_free_inodes_group.top < 0) {
            // need to get next group
            read_block(&sb_info_p->s_free_inodes_group, inode_addr, sizeof(struct free_inodes_group), 1, v_device);
            // write back to first free inode block
            write2block(&sb_info_p->s_free_inodes_group, sb_info_p->s_free_inodesmg_block, sizeof(struct free_inodes_group), 1, v_device);
        }
        ino = inode_addr - sb_info_p->s_first_inode_block;      // ino >= 0 here && < UINT32_MAX
    }
    return ino;                                 // return UINT32_MAX if not enough inode
}

/* beanfs_inode to beanfs_inode_info
 */
int beanfs_transform2inode_info(struct beanfs_inode *inode_p, struct beanfs_inode_info *inode_info_p, uint32_t ino)
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
        inode_info_p->i_ino = ino;
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

int update_inode(struct beanfs_sb_info *sb_info_p, struct beanfs_inode_info *inode_info_p,
                 struct beanfs_inode *inode_p, FILE *v_device)
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
        status = beanfs_write_inode(sb_info_p, inode_p, inode_info_p->i_ino + sb_info_p->s_first_inode_block, v_device);
    } else {
        status = -1;
    }
    return status;

}

uint32_t beanfs_alloc_inode(struct beanfs_sb_info *sb_info_p, struct beanfs_inode_info *ei ,FILE *v_device)
{
    uint32_t ino = UINT32_MAX;
    //struct beanfs_inode_info *ei = NULL;
    if (sb_info_p == NULL || v_device == NULL) {
        return ino;
    }
    if (sb_info_p->s_free_inodes_count > 0) {
        //allocate inode assert free inode count > 0 then do getinode
        ino = beanfs_get_inode(sb_info_p, v_device);
        //ei = (struct beanfs_inode_info *)malloc(sizeof(struct beanfs_inode_info));
        init_inode_info(ei, ino);
    }
    
    return ino;          // return inode number if success
                         // return UINT32_MAX if allocation failed
}

int beanfs_i_callback(struct beanfs_sb_info *sb_info_p, struct beanfs_inode_info *callback_i, FILE *v_device)
{
    int status = -1;
    uint32_t inode_addr = 0;
    // struct free_inodes_group tmp_inodes_group = {0, {0}};
    if (sb_info_p != NULL && callback_i != NULL && v_device != NULL) {
        // callback the inode
        inode_addr = sb_info_p->s_first_inode_block + callback_i->i_ino;
        
        if (sb_info_p->s_free_inodes_group.top < FREE_INODES_LIST_SIZE) {
            sb_info_p->s_free_inodes_group.list[++(sb_info_p->s_free_inodes_group.top)] = inode_addr;
        } else {
            // first inode group is full then create a new inode group as first inode group
            write2block(&sb_info_p->s_free_inodes_group, inode_addr, sizeof(struct free_inodes_group), 1, v_device);
            sb_info_p->s_free_inodes_group.top = 0;
            sb_info_p->s_free_inodes_group.list[0] = inode_addr;
            //tmp_inodes_group.list[0] = sb_info_p->s_free_inodes_group.list[0];
            //sb_info_p->s_free_inodes_group = tmp_inodes_group;
            
        }
        sb_info_p->s_free_inodes_count++;
        // write back to disk
        write2block(&sb_info_p->s_free_inodes_group, sb_info_p->s_free_inodesmg_block, sizeof(struct free_inodes_group), 1, v_device);
        status = 1;                                                                 // callback success
    }
    return status;
}