//
//  commands.c
//  v_filesystem
//
//  Created by WONDER on 1/15/14.
//  Copyright (c) 2014 WONDER. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "beanfs.h"
#include "commands.h"

int beanfs_pwd(struct envrioment_variable *envvars_p)
{
    int status = 0;
    if (strcmp(envvars_p->command, "pwd") == 0) {
        printf(" %s \n", envvars_p->curdir);
    }
    return status;
}

int beanfs_cd(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    int status = 0;
    struct beanfs_dir_entry tmp_entry;
    char *last_slash = NULL;
    //char tmp_argv[100];
    
    if (strcmp(envvars_p->command, "cd") == 0) {
        if (envvars_p->argc == 2) {
            if (envvars_p->argv[1][0] != '/') {
                printf(" invalid path \n");
                return status;
            }
            if (beanfs_lookup(envvars_p->argv[1], sb_info_p, &tmp_entry, v_device) && tmp_entry.d_file_type == 'd') {
                if (strcmp(tmp_entry.d_name, ".") != 0 && strcmp(tmp_entry.d_name, "..") != 0) {
                    strcpy(envvars_p->curdir, envvars_p->argv[1]);
                } else if (strcmp(tmp_entry.d_name, "..") == 0){
                    if (strcmp(envvars_p->curdir, "/") != 1) {
                        last_slash = strrchr(envvars_p->curdir, '/');
                        if (last_slash != envvars_p->curdir) {
                            last_slash[1] = '\0';
                        } else {
                            (envvars_p->curdir)[1] = '\0';
                        }
                    }
                }
            } else {
                fprintf(stderr, " %s : doesn't exist \n", envvars_p->argv[1]);
            }
        }
    }
    return status;
}

int beanfs_ls(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    int status = 0;
    struct beanfs_dir dir;
    struct beanfs_dir_entry entry;
    struct beanfs_inode tmp_inode;
    if (envvars_p->argc > 1) {
        if (beanfs_lookup(envvars_p->argv[0], sb_info_p, &entry, v_device) == 1) {
            if (entry.d_file_type == 'd') {
                // get dir info
                beanfs_read_inode(sb_info_p, &tmp_inode, sb_info_p->s_first_inode_block + entry.d_ino, v_device);
                read_block(&dir, tmp_inode.i_addr.d_addr[0], sizeof(struct beanfs_dir), 1, v_device);
                for (uint8_t i = 8; i < dir.len; i++) {
                    printf("%s  ", dir.entrys[i].d_name);
                }
                printf("\n");
            } else {
                printf("%s  \n", entry.d_name);
            }
        }
    } else {
        if (strcmp(envvars_p->curdir, "/")) {
            read_block(&dir, sb_info_p->s_first_data_block, sizeof(struct beanfs_dir), 1, v_device);
            for (uint8_t i = 0; i < dir.len; i++) {
                printf("%s  ", dir.entrys[i].d_name);
            }
        } else {
            beanfs_lookup(envvars_p->curdir, sb_info_p, &entry, v_device);
            beanfs_read_inode(sb_info_p, &tmp_inode, sb_info_p->s_first_inode_block + entry.d_ino, v_device);
            read_block(&dir, tmp_inode.i_addr.d_addr[0], sizeof(struct beanfs_dir), 1, v_device);
            for (uint8_t i = 0; i < dir.len; i++) {
                printf("%s  ", dir.entrys[i].d_name);
            }
        }
        printf("\n");
    }
    return status;
}

int beanfs_mkdir(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    int status = 0;
    uint32_t ino = UINT32_MAX;
    struct beanfs_inode_info dir_inode_info;
    struct beanfs_inode dir_inode;
    struct beanfs_dir_entry new_entry;
    struct beanfs_dir new_dir;
    new_dir.len = 2;
    
    struct beanfs_inode r_inode;
    struct beanfs_inode_info r_inode_info;
    beanfs_read_inode(sb_info_p, &r_inode, sb_info_p->s_first_inode_block, v_device);
    beanfs_transform2inode_info(&r_inode, &r_inode_info, 0);
    
    ino = beanfs_alloc_inode(sb_info_p, &dir_inode_info, v_device);
    new_entry.d_file_type = 'd';
    new_entry.d_ino = ino;
    strcpy(new_entry.d_name, "etc");
    beanfs_add_entry(&new_entry, sb_info_p, &r_inode_info, v_device);
    dir_inode_info.i_addr.d_addr[0] = beanfs_alloc_datablock(sb_info_p, v_device);
    new_dir.entrys[0].d_file_type = 'd';
    new_dir.entrys[0].d_ino = ino;
    strcpy(new_dir.entrys[0].d_name, ".");
    new_dir.entrys[1].d_file_type = 'd';
    new_dir.entrys[1].d_ino = 0;
    strcpy(new_dir.entrys[1].d_name, "..");
    write2block(&new_dir, dir_inode_info.i_addr.d_addr[0], sizeof(struct beanfs_dir), 1, v_device);
    update_inode(sb_info_p, &dir_inode_info, &dir_inode, v_device);
    return status;
}