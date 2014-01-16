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


int beanfs_mkfs(FILE *v_device)
{
    int blocks = 0;
    const char command[] = "dd if=/dev/zero of=virtual_device bs=512 count=";
    char blocks_str[10];
    char full_command[100];
    int status = 0;
    //FILE *v_device = NULL;
    
    do {
        printf("please enter block number: (>= 10 block)  to initialize this system \n");
        scanf("%s", blocks_str);
        blocks = atoi(blocks_str);
    } while (blocks < 10);
    
    strcpy(full_command, command);
    strcat(full_command, blocks_str);
    system(full_command);
    
    v_device = fopen("virtual_device", "ab+");
    if (v_device != NULL) {
        status = init_beanfs(blocks, v_device);
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

static void beanfs_login(struct envrioment_variable *envvars_p)
{
    const char account_file_name[] = "account_file";
    FILE *account_file;
    char username[20] = {0};
    int uid = 0;
    char passwd[20];
    int status = 0;
    char input_username[20] = {0};
    char input_passwd[20];
    int flag = 1;
    
    account_file = fopen(account_file_name, "r");
    status = fscanf(account_file, "%s %d %s\n", username, &uid, passwd);
    //printf("%s %d %s", username, uid, passwd);
    if (status == 3) {
        do {
            printf("username: ");
            scanf("%s", input_username);
            printf("password: ");
            scanf(" %s", input_passwd);
            if (strcmp(input_username, username) == 0 && strcmp(input_passwd, passwd) == 0) {
                flag = 0;
            } else {
                printf(" wrong username and password \n");
            }
        } while (flag);
    } else {
        fclose(account_file);
        fprintf(stderr, "error with account_file");
        exit(1);
    }
    envvars_p->gid = 0;
    strcpy(envvars_p->user, username);
    envvars_p->uid = uid;
    strcpy(envvars_p->curdir, "/");
    fclose(account_file);
}

void beanfs_shell(FILE *v_device)
{
    char *input = NULL;
    size_t len = 0;
    struct beanfs_super_block super_block;
    struct beanfs_sb_info sb_info;
    char curdir_buffer[BLOCK_SIZE] = {0};
    struct envrioment_variable envvars;
    envvars.curdir = curdir_buffer;
    
    // init enviroment
    system("clear");
    beanfs_login(&envvars);
    read_superblock(&super_block, v_device);
    beanfs_transform2sb_info(&super_block, &sb_info, v_device);
    
    while (1) {
        // first exetue is to get rid of the
        printf("%s @ virtual_filesystem:%s \n", envvars.user, envvars.curdir);
        len = getline(&input, &len, stdin);
        input[len - 1] = '\0';
        parse_shell_input(input, &envvars.command, &envvars.argc, envvars.argv);
        if (envvars.argc > 0) {
            if (strcmp(envvars.command, "exit") == 0) {
                break;
            } else if (strcmp(envvars.command, "ls") == 0) {
                beanfs_ls(&envvars, &sb_info, v_device);
            } else if (strcmp(envvars.command, "cd") == 0) {
                beanfs_cd(&envvars, &sb_info, v_device);
            } else if (strcmp(envvars.command, "mv") == 0) {
                
            } else if (strcmp(envvars.command, "cp") == 0) {
            
            } else if (strcmp(envvars.command, "ln") == 0) {
                
            } else if (strcmp(envvars.command, "rm") == 0) {
            
            } else if (strcmp(envvars.command, "pwd") == 0) {
                beanfs_pwd(&envvars);
            } else if (strcmp(envvars.command, "cat") == 0) {
            
            } else if (strcmp(envvars.command, "mkdir") == 0) {
                beanfs_mkdir(&envvars, &sb_info, v_device);
            } else if (strcmp(envvars.command, "rmdir") == 0) {
                
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
}