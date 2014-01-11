//
//  dir.c
//  v_filesystem
//
//  Created by WONDER on 12/27/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#include <assert.h>
#include <string.h>
#include "beanfs.h"

int beanfs_add_entry(struct beanfs_dir_entry *new_entry, struct beanfs_sb_info *sb_info_p,
                     struct beanfs_inode_info *d_inode_p, FILE *v_device)
{
    int status = -1;
    struct beanfs_dir curdir = {0};
    uint32_t dir_addr = UINT32_MAX;
    int8_t entrys_count = 0;
    // for convience, there must 1 block recording entry_information for every dir
    // that is every directory only have limited entry (18 entrys)
    dir_addr = d_inode_p->i_addr.d_addr[0];
    assert(d_inode_p->i_blocks == 1);
    assert(dir_addr >= sb_info_p->s_first_data_block && dir_addr < sb_info_p->s_blocks_count);
    status = read_block(&curdir, dir_addr, sizeof(struct beanfs_dir), 1, v_device);
    if (status > 0) {
        // begin add entry
        if (curdir.len < MAX_ENTRY) {
            for (entrys_count = 0; entrys_count < curdir.len; entrys_count++) {
                if (strcmp(curdir.entrys[entrys_count].d_name, new_entry->d_name) == 0) {
                    // same name in the entry
                    fprintf(stderr, "FILE %s exists \n", new_entry->d_name);
                    return status;
                }
            }
            curdir.entrys[curdir.len++] = *new_entry;
            status = write2block(&curdir, dir_addr, sizeof(struct beanfs_dir), 1, v_device);
            if (status > 0) {
                status = 1;
            } else {
                status = -1;
            }
        } else {
            fprintf(stderr, "this directory has reached the limitation of entrys \n");
        }
    }
    
    return status;
}


