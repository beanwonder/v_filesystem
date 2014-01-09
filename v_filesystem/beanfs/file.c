//
//  file.c
//  v_filesystem
//
//  Created by WONDER on 12/27/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#include <string.h>
#include "beanfs.h"

int beanfs_create_file(const char filename[], struct beanfs_sb_info *sb_info_p, struct beanfs_inode_info *dir_inode_p, FILE *v_device)
{
    int status = -1;
    struct beanfs_dir dirinfo;
    
    if (sb_info_p == NULL || dir_inode_p == NULL) {
        return status;
    }
    if (dir_inode_p->i_blocks > 0 && dir_inode_p->i_blocks <= DIR_MAX_BLOCK) {
        for (int i = 0; i < dir_inode_p->i_blocks; i++) {
            read_data_block(sb_info_p, &dirinfo, dir_inode_p->i_addr.d_addr[i], v_device);
            for (uint8_t i = 0; i < dirinfo.len; i++) {
                if (strcmp(filename, dirinfo.entrys[i].d_name) == 0) {
                    fprintf(stderr, "file name %s has exists \n", filename);
                    return status;
                }
            }
        }
        // if run until here, comes the last block recording dir information
        if (dirinfo.len < MAX_ENTRY) {
            <#statements#>
        }
        
    }
    
    return status;
}