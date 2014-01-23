//
//  util.c
//  v_filesystem
//
//  Created by WONDER on 12/26/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "beanfs.h"
#include "util.h"
#include "commands.h"


int beanfs_mkfs(const char vfs_device[])
{
    int blocks = 0;
    const char command[] = "dd if=/dev/zero of=virtual_device bs=512 count=";
    char blocks_str[10];
    char full_command[100];
    int status = 0;
    FILE *v_device = NULL;
    
    do {
        printf("please enter block number: (>= 10 block)  to initialize this system \n");
        scanf("%s", blocks_str);
        blocks = atoi(blocks_str);
    } while (blocks < 10);
    
    printf(" creating %d blocks virtual_filesystem \n", blocks);
    
    
    strcpy(full_command, command);
    strcat(full_command, blocks_str);
    system(full_command);
    
    v_device = fopen("virtual_device", "rb+");
    if (v_device != NULL) {
        status = init_beanfs(blocks, v_device);
        fclose(v_device);
            // create /passwd file
    }
    return blocks;
}

static void parse_shell_input(char *input, char **command, int *argc, char *argv[])
{
    char *blank_pos = NULL;
    char *search_pos = NULL;
    const char blank_sep[] = " ";
    
    // init
    *command = input;
    *argc = 0;
    argv[0] = NULL;
    search_pos = input;
    blank_pos = input;

    while (blank_pos != NULL && input[0] != '\0') {
        blank_pos = strpbrk(search_pos, blank_sep);
        argv[(*argc)++] = search_pos;
        
        if (blank_pos) {
            // when blank_pos != NULL
            *blank_pos = '\0';
            search_pos = blank_pos + 1;
        }
    }
    assert(*argc < 100);
}

static void beanfs_login(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device)
{
    char passwd_full_path[] = "/passwd";
    char username[20] = {0};
    char passwd[20] = {0};
    char input_username[20] = {0};
    char input_passwd[20];
    int flag = 1;
    char buffer[BLOCK_SIZE] = {0};
    struct beanfs_inode passwd_inode;
    struct beanfs_inode_info passwd_inode_info;
    struct beanfs_dir root_dir;
    struct beanfs_dir_entry passwd_entry;
    char *start = NULL;
    char *end = NULL;
    
    struct beanfs_inode_info root_inode_info;
    struct beanfs_inode root_inode;
    read_block(&root_dir, sb_info_p->s_first_data_block, sizeof(struct beanfs_dir), 1, v_device);
    beanfs_read_inode(sb_info_p, &root_inode, sb_info_p->s_first_inode_block, v_device);
    beanfs_transform2inode_info(&root_inode, &root_inode_info, 0);
    
    //printf("%s %d %s", username, uid, passwd);
    if (beanfs_lookup(passwd_full_path, sb_info_p, &passwd_entry, v_device) == 1) {
        beanfs_read_inode(sb_info_p, &passwd_inode, sb_info_p->s_first_inode_block + passwd_entry.d_ino, v_device);
        read_data_block(sb_info_p, buffer, passwd_inode.i_addr.d_addr[0], v_device);
        start = buffer;
        end = strpbrk(start, " ");
        strncpy(username, start, end - start);
        username[end - start] = '\0';
        start = end + 1;
        end = strpbrk(start, " ");
        strncpy(passwd, start, end - start);
        passwd[end - start] = '\0';
        start = end + 1;
        
        do {
            printf("username: ");
            scanf("%s", input_username);
            printf("password: ");
            scanf(" %s", input_passwd);
            getchar();
            if (strcmp(input_username, username) == 0 && strcmp(input_passwd, passwd) == 0) {
                flag = 0;
            } else {
                printf(" wrong username and password \n");
            }
        } while (flag);
    } else {
        // passwd doesn't exits
        // create a passwd file
        //struct beanfs_dir_entry passwd_entry;
        
        printf("creating new account . . . \n");
        
        printf("enter a username: ");
        scanf("%s", input_username);
        printf("enter a password: ");
        scanf(" %s", input_passwd);
        getchar();
        strcpy(username, input_username);
        strcpy(passwd, input_passwd);
        
        strcpy(passwd_entry.d_name, "passwd");
        passwd_entry.d_file_type = '-';
        passwd_entry.d_ino = beanfs_alloc_inode(sb_info_p, &passwd_inode_info, v_device);
        beanfs_add_entry(&passwd_entry, sb_info_p, &root_inode_info, v_device);
        passwd_inode_info.i_uid = 0;
        passwd_inode_info.i_mode = 644;
        passwd_inode_info.i_blocks = 1;
        passwd_inode_info.i_links = 0;
        passwd_inode_info.i_mode = 0644;
        passwd_inode_info.i_addr.d_addr[0] = beanfs_alloc_datablock(sb_info_p, v_device);
        
        strcpy(buffer, username);
        strcat(buffer, " ");
        strcat(buffer, passwd);
        strcat(buffer, " ");
        write2block(buffer, passwd_inode_info.i_addr.d_addr[0], BLOCK_SIZE, 1, v_device);
        update_inode(sb_info_p, &passwd_inode_info, &passwd_inode, v_device);
    }
    
    envvars_p->gid = 0;
    strcpy(envvars_p->user, username);
    envvars_p->uid = 0;
    strcpy(envvars_p->curdir, "/");
}

void beanfs_shell(const char vfs_device[])
{
    char *input = NULL;
    size_t len = 0;
    struct beanfs_super_block super_block;
    struct beanfs_sb_info sb_info;
    FILE *v_device = NULL;
    char curdir_buffer[BLOCK_SIZE] = {0};
    struct envrioment_variable envvars;
    envvars.curdir = curdir_buffer;
    
    v_device = fopen(vfs_device, "rb+");
    assert(v_device);
    // init enviroment
    if (v_device != NULL) {
        system("clear");
        read_superblock(&super_block, v_device);
        beanfs_transform2sb_info(&super_block, &sb_info, v_device);
        beanfs_login(&envvars, &sb_info, v_device);
        
        while (1) {
            // first exetue is to get rid of the
            printf("%s @ virtual_filesystem:%s \n", envvars.user, envvars.curdir);
            len = getline(&input, &len, stdin);
            input[len - 1] = '\0';
            parse_shell_input(input, &envvars.command, &envvars.argc, envvars.argv);
            if (envvars.argc > 0) {
                if (strcmp(envvars.command, "exit") == 0) {
                    // sync superblock before exit
                    update_superblock(&sb_info, &super_block, v_device);
                    break;
                } else if (strcmp(envvars.command, "ls") == 0) {
                    beanfs_ls(&envvars, &sb_info, v_device);
                } else if (strcmp(envvars.command, "cd") == 0) {
                    beanfs_cd(&envvars, &sb_info, v_device);
                } else if (strcmp(envvars.command, "mv") == 0) {
                    
                } else if (strcmp(envvars.command, "cp") == 0) {
                
                } else if (strcmp(envvars.command, "ln") == 0) {
                    
                } else if (strcmp(envvars.command, "pwd") == 0) {
                    beanfs_pwd(&envvars);
                } else if (strcmp(envvars.command, "cat") == 0) {
                
                } else if (strcmp(envvars.command, "mkdir") == 0) {
                    beanfs_mkdir(&envvars, &sb_info, v_device);
                    update_superblock(&sb_info, &super_block, v_device);
                } else if (strcmp(envvars.command, "clear") == 0) {
                    beanfs_clear();
                } else if (strcmp(envvars.command, "rmdir") == 0) {
                    beanfs_rmdir(&envvars, &sb_info, v_device);
                    update_superblock(&sb_info, &super_block, v_device);
                } else if (strcmp(envvars.command, "passwd") == 0) {
                
                } else {
                    fprintf(stderr, "%s: no such command \n", envvars.command);
                }
            }
        }
        if (input) {
            free(input);
        }
        fclose(v_device);
    } else {
        fprintf(stderr, "an error occured, file: %s doesn't exists \n", vfs_device);
        exit(1);
    }
}