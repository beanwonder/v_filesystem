//
//  main.c
//  v_filesystem
//
//  Created by WONDER on 12/5/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"
#include "beanfs.h"

int main(void)
{
    int vfs_exist = -1;
    const char vfs_device[] = "virtual_device";
    void show_main_menu();
    char choice = '\0';
    char terminater = '\0';
    
    while (1) {
        vfs_exist = access(vfs_device, F_OK);
        show_main_menu(vfs_exist);
        printf("    please choose a number \n");
        
        if (!vfs_exist) {
            do {
                choice = getchar();
                terminater = getchar();
            } while (choice != '1' && choice != '2' && choice != '3');
            
            switch (choice) {
                case '1':
                    printf("creating a filesystem \n");
                    beanfs_mkfs(vfs_device);
                    beanfs_shell(vfs_device);
                    //create_filesystem();
                    break;
                case '2':
                    printf("accessing existed filesystem\n");
                    beanfs_shell(vfs_device);
                    //access_exist_filesystem();
                    break;
                case '3':
                    printf("leaving, bye! \n");
                    exit(0);
                    break;
                default:
                    break;
            }
        } else {
            do {
                choice = getchar();
                terminater = getchar();
            } while (choice != '1' && choice != '2');
            
            switch (choice) {
                case '1':
                    printf("createing a filesystem \n");
                    beanfs_mkfs(vfs_device);
                    beanfs_shell(vfs_device);
                    //create_filesystem();
                    break;
                case '2':
                    printf("leaving, bye! \n");
                    exit(0);
                    break;
                default:
                    break;
            }
        }
        system("clear");
    }
    return 0;
}

void show_main_menu(int vfs_exists)
{
    printf("*****************************************************\n");
    printf("*   welcome to this unix-like virtual file system   *\n");
    printf("*              here is the main memu                *\n");
    printf("*****************************************************\n");
    
    if (!vfs_exists) {
        printf("*          1. create a brand new file system        *\n");
        printf("*          2. access existing file system           *\n");
        printf("*          3. exit                                  *\n");
        printf("*****************************************************\n");
    } else {
        printf("*          1. create a brand new file system        *\n");
        printf("*          2. exit                                  *\n");
        printf("*****************************************************\n");
    }
}