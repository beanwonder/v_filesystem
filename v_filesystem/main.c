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
    struct beanfs_super_block superblock;
    struct beanfs_inode root_inode;
    struct beanfs_dir root_dir;
    
    FILE *virtual_device = NULL;
    system("dd if=/dev/zero of=virtual_device bs=512 count=100");
    virtual_device = fopen("virtual_device", "wb+");
    init_beanfs(100, virtual_device);
    printf("superblock resutl %d\n", read_superblock(&superblock, virtual_device));
    printf("root inode result %d\n" ,read_block(&root_inode, superblock.s_first_inode_block, sizeof(struct beanfs_inode), 1, virtual_device));
    printf("root dir resutl %d\n", read_block(&root_dir, root_inode.i_addr.d_addr[0], sizeof(struct beanfs_dir), 1, virtual_device));
    printf("rootdir len %d", root_dir.len);
    for (int i = 0; i < root_dir.len; i++) {
        printf("filename %s ino %d \n", root_dir.entrys[i].d_name, root_dir.entrys[i].d_ino);
    }
    fclose(virtual_device);
    // -------------------------------------------------------
    // -------------------------------------------------------
    while (1) {
        vfs_exist = access(vfs_device, F_OK);
        show_main_menu(vfs_exist);
        printf("    please choose a number \n");
        
        if (vfs_exist == 0) {
            do {
                choice = getchar();
            } while (choice != '1' && choice != '2' && choice != '3');
            
            switch (choice) {
                case '1':
                    printf("creating a filesystem \n");
                    //create_filesystem();
                    break;
                case '2':
                    printf("accessing existed filesystem\n");
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
            } while (choice != '1' && choice != '2');
            
            switch (choice) {
                case '1':
                    printf("createing a filesystem \n");
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
    
    if (vfs_exists == 0) {
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