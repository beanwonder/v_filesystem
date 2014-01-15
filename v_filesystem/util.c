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
#include "beanfs.h"
#include "util.h"

int beanfs_mkfs(FILE *v_device)
{
    int blocks = 0;
    const char command[] = "dd if=/dev/zero of=virtual_device bs=512 count=100";
    char blocks_str[10];
    char full_command[100];
    int status = 0;
    
    do {
        printf("please enter block number: ( > 10 blocks) \n");
        scanf("%s", blocks_str);
        blocks = atoi(blocks_str);
    } while (blocks > 10);
    
    strcpy(full_command, command);
    strcat(full_command, blocks_str);
    system(full_command);
    status = init_beanfs(blocks, v_device);
    
    return blocks;
}

static void parse_shell_input(char *input)
{
    
}

void beanfs_shell(FILE *v_device)
{
    char *input = NULL;
    size_t len = 0;
    
    while (1) {
        getline(&input, &len, stdin);
        printf("input : %s \n", input);
        break;
    }
    if (input) {
        free(input);
    }
}