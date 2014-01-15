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
#include "beanfs.h"
#include "util.h"
#include "commands.h"

#define MAX_USER_NAME 100
#define MAX_ARGC 100

struct envrioment_variable {
    char user[MAX_USER_NAME];
    uint32_t uid;
    uint32_t gid;
    char *curdir;
    char *command;
    int argc;
    char *argv[MAX_ARGC];
};

int beanfs_mkfs(const char vfs_device[])
{
    FILE *v_device = NULL;
    int blocks = 0;
    const char command[] = "dd if=/dev/zero of=virtual_device bs=512 count=";
    char blocks_str[10];
    char full_command[100];
    int status = 0;
    
    do {
        printf("please enter block number: (>= 10 block)  to initialize this system \n");
        scanf("%s", blocks_str);
        blocks = atoi(blocks_str);
    } while (blocks < 10);
    
    strcpy(full_command, command);
    strcat(full_command, blocks_str);
    system(full_command);
    
    v_device = fopen(vfs_device, "ab+");
    if (v_device != NULL) {
        status = init_beanfs(blocks, v_device);
        fclose(v_device);
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

static void beanfs_login(struct envrioment_variable * envvars, FILE *v_device)
{
    
}

void beanfs_shell(const char vfs_device[])
{
    FILE *v_device = NULL;
    char *input = NULL;
    size_t len = 0;
    struct envrioment_variable envvars = {"root", 0, 0, "/"};
    // init enviroment
    v_device = fopen(vfs_device, "ab+");
    assert(vfs_device != NULL);
    system("clear");
    beanfs_login(&envvars, v_device);
    
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
                
            } else if (strcmp(envvars.command, "cd")) {
                
            } else if (strcmp(envvars.command, "mv")) {
                
            } else if (strcmp(envvars.command, "cp")) {
            
            } else if (strcmp(envvars.command, "ln")) {
                
            } else if (strcmp(envvars.command, "rm")) {
            
            } else if (strcmp(envvars.command, "pwd")) {
                
            } else if (strcmp(envvars.command, "cat")) {
            
            } else if (strcmp(envvars.command, "mkdir")) {

            } else if (strcmp(envvars.command, "rmdir")) {

            } else if (strcmp(envvars.command, "passwd")) {
            
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