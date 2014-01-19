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
    //char tmp_argv[100];
    char full_path_tmp[BLOCK_SIZE] = {0};
    
    if (envvars_p->argc > 1) {
        if (envvars_p->argv[1][0] != '/') {
            //
            if (strcmp(envvars_p->curdir, "/") != 0) {
                strcat(full_path_tmp, envvars_p->curdir);
            }
            strcat(full_path_tmp, "/");
            strcat(full_path_tmp, envvars_p->argv[1]);
        } else {
            // absolute path
            strcpy(full_path_tmp, envvars_p->argv[1]);
        }
        if (beanfs_lookup(full_path_tmp, sb_info_p, &tmp_entry, v_device) == 1 && tmp_entry.d_file_type == 'd') {
            strcpy(envvars_p->curdir, full_path_tmp);
            status = 1;
        } else {
            fprintf(stderr, "cd: %s is not a directory \n", envvars_p->argv[1]);
        }
    } else {
        // keep keep currentdir
        status = 1;
    }
    return status;
}

int beanfs_ls(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    int status = 0;
    struct beanfs_dir tmp_dir;
    struct beanfs_dir_entry tmp_entry;
    struct beanfs_inode tmp_inode;
    char abs_path[BLOCK_SIZE] = {0};
    
    if (envvars_p->argc == 2) {
        if (envvars_p->argv[1][0] != '/') {
            if (strcmp(envvars_p->curdir, "/") != 0) {
                strcpy(abs_path, envvars_p->argv[1]);
            }
            strcat(abs_path, "/");
            strcat(abs_path, envvars_p->argv[1]);
        } else {
            strcpy(abs_path, envvars_p->argv[1]);
        }
    } else {
        strcpy(abs_path, envvars_p->curdir);
    }
    
    if (beanfs_lookup(abs_path, sb_info_p, &tmp_entry, v_device) == 1) {
        if (tmp_entry.d_file_type == 'd') {
            // get dir information
            beanfs_read_inode(sb_info_p, &tmp_inode, sb_info_p->s_first_inode_block + tmp_entry.d_ino, v_device);
            read_block(&tmp_dir, tmp_inode.i_addr.d_addr[0], sizeof(struct beanfs_dir), 1, v_device);
            for (int8_t i = 0; i < tmp_dir.len; i++) {
                printf("%s ", tmp_dir.entrys[i].d_name);
            }
            printf("\n");
            status = 1;
        } else {
            printf("type: %c  %s \n", tmp_entry.d_file_type, tmp_entry.d_name);
            status = 1;
        }
    } else {
        fprintf(stderr, " %s doesn't exits \n", envvars_p->argv[1]);
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

int beanfs_clear()
{
    system("clear");
    return 0;
}