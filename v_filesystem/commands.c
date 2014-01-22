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
    //uint32_t ino = UINT32_MAX;
    struct beanfs_inode_info dir_inode_info;
    struct beanfs_inode dir_inode;
    struct beanfs_dir_entry new_entry;
    struct beanfs_dir_entry par_entry;
    //struct beanfs_dir_entry tmp_entry;
    struct beanfs_dir new_dir = {.len = 2};
    char full_path[BLOCK_SIZE] = {0};
    char dname[BLOCK_SIZE] = {0};
    
    new_dir.entrys[0].d_file_type = 'd';
    strcpy(new_dir.entrys[0].d_name, ".");
    new_dir.entrys[1].d_file_type = 'd';
    strcpy(new_dir.entrys[1].d_name, "..");
    
    if (envvars_p->argc == 2) {
        strcpy(full_path, envvars_p->curdir);
        strcpy(dname, envvars_p->argv[1]);
    } else if (envvars_p->argc == 3) {
        strcpy(full_path, envvars_p->argv[1]);
        strcpy(dname, envvars_p->argv[2]);
    } else {
        fprintf(stderr, "wrong directory \n");
        return status;
    }
    
    if (beanfs_lookup(full_path, sb_info_p, &par_entry, v_device) == 1 && par_entry.d_file_type == 'd') {
        //strcat(full_path, "/");
        strcat(full_path, dname);
        //printf("%s \n", full_path);
        if (beanfs_lookup(full_path, sb_info_p, &new_entry, v_device) == -1) {
            struct beanfs_inode par_inode;
            struct beanfs_inode_info par_inode_info;
            beanfs_read_inode(sb_info_p, &par_inode, sb_info_p->s_first_inode_block + par_entry.d_ino, v_device);
            beanfs_transform2inode_info(&par_inode, &par_inode_info, par_entry.d_ino);
            new_entry.d_file_type = 'd';
            strcpy(new_entry.d_name, dname);
            new_entry.d_ino = beanfs_alloc_inode(sb_info_p, &dir_inode_info, v_device);
            beanfs_add_entry(&new_entry, sb_info_p, &par_inode_info, v_device);
            new_dir.entrys[0].d_ino = new_entry.d_ino;
            new_dir.entrys[1].d_ino = par_entry.d_ino;
            dir_inode_info.i_blocks = 1;
            dir_inode_info.i_addr.d_addr[0] = beanfs_alloc_datablock(sb_info_p, v_device);
            update_inode(sb_info_p, &dir_inode_info, &dir_inode, v_device);
            write2block(&new_dir, dir_inode_info.i_addr.d_addr[0], sizeof(struct beanfs_dir), 1, v_device);
            status = 1;
        } else {
            fprintf(stderr, " %s has exists \n", dname);
        }
    } else {
        fprintf(stderr, " %s is not a directory \n", full_path);
    }
    
    return status;
}

int beanfs_rmdir(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    int status = 0;
    char dname[BLOCK_SIZE] = {0};
    char full_path[BLOCK_SIZE] = {0};
    struct beanfs_dir_entry dir_entry;
    struct beanfs_inode dir_inode;
    struct beanfs_inode_info dir_inode_info;
    
    struct beanfs_inode pdir_inode;
    struct beanfs_inode_info pdir_inode_info;
    
    if (envvars_p->argc == 2) {
        if (envvars_p->argv[1][0] != '/') {
            strcpy(dname, envvars_p->argv[1]);
            strcpy(full_path, envvars_p->curdir);
            if (strcmp(envvars_p->curdir, "/") != 0) {
                strcat(full_path, "/");
            }
            strcat(full_path, dname);
            // begin remove
            if (beanfs_lookup(full_path, sb_info_p, &dir_entry, v_device) == 1 && dir_entry.d_file_type == 'd') {
                if (strcmp(envvars_p->curdir, "/") != 0) {
                    struct beanfs_dir_entry par_entry;
                    beanfs_lookup(envvars_p->curdir, sb_info_p, &par_entry, v_device);
                    beanfs_read_inode(sb_info_p, &pdir_inode, sb_info_p->s_first_inode_block + par_entry.d_ino, v_device);
                    beanfs_transform2inode_info(&pdir_inode, &pdir_inode_info, par_entry.d_ino);
                } else {
                    // read '/' inode info
                    beanfs_read_inode(sb_info_p, &pdir_inode, sb_info_p->s_first_inode_block, v_device);
                    beanfs_transform2inode_info(&pdir_inode, &pdir_inode_info, 0);
                }
                beanfs_read_inode(sb_info_p, &dir_inode, sb_info_p->s_first_inode_block + dir_entry.d_ino, v_device);
                beanfs_transform2inode_info(&dir_inode, &dir_inode_info, dir_entry.d_ino);
                if (dir_inode.i_links <= 1) {
                    struct beanfs_dir delete_dir;
                    struct beanfs_dir_entry deleted_entry;
                    struct beanfs_dir tmpdir;
                    struct beanfs_inode tmpinode;
                    //char blank_block[BLOCK_SIZE] = {0};
                    read_block(&delete_dir, dir_inode_info.i_addr.d_addr[0], sizeof(struct beanfs_dir), 1, v_device);
                    if (delete_dir.len <= 2) {
                        // empty dir
                        read_block(&tmpdir, sb_info_p->s_first_data_block, sizeof(struct beanfs_dir), 1, v_device);
                        beanfs_read_inode(sb_info_p, &tmpinode, sb_info_p->s_first_inode_block, v_device);
                        beanfs_callback_datablock(sb_info_p, dir_inode_info.i_addr.d_addr[0], v_device);
                        
                        beanfs_read_inode(sb_info_p, &tmpinode, sb_info_p->s_first_inode_block, v_device);
                        read_block(&tmpdir, sb_info_p->s_first_data_block, sizeof(struct beanfs_dir), 1, v_device);
                        beanfs_delete_entry(dname, sb_info_p, &pdir_inode_info, &deleted_entry, v_device);
                        
                        read_block(&tmpdir, sb_info_p->s_first_data_block, sizeof(struct beanfs_dir), 1, v_device);
                        beanfs_read_inode(sb_info_p, &tmpinode, sb_info_p->s_first_inode_block, v_device);
                        beanfs_i_callback(sb_info_p, &dir_inode_info, v_device);
                    
                        read_block(&tmpdir, sb_info_p->s_first_data_block, sizeof(struct beanfs_dir), 1, v_device);
                        beanfs_read_inode(sb_info_p, &tmpinode, sb_info_p->s_first_inode_block, v_device);

                        status = 1;
                    } else {
                        fprintf(stderr, "%s : is not empty \n", full_path);
                    }
                } else {
                    // delete links
                    struct beanfs_dir_entry deleted_entry;
                    beanfs_delete_entry(dname, sb_info_p, &pdir_inode_info, &deleted_entry, v_device);
                    dir_inode.i_links--;
                    status = 1;
                }
            } else {
                fprintf(stderr, "%s: is a directory \n", full_path);
            }
            
        } else {
            fprintf(stderr, "ilegal input \n");
        }
    } else {
        fprintf(stderr, "ilegal input \n");
    }
    return status;
}

int beanfs_clear()
{
    system("clear");
    return 0;
}