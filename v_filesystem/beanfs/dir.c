//
//  dir.c
//  v_filesystem
//
//  Created by WONDER on 12/27/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#include <assert.h>
#include <string.h>
#include "beanfs.h"

static int search_dir_entry(char search_fname[], struct beanfs_dir *search_dir_p, struct beanfs_dir_entry *entry_p)
{
    int search_status = 0;
    for (int8_t i = 0; i < search_dir_p->len; i++) {
        if (strcmp(search_fname, search_dir_p->entrys[i].d_name) == 0) {
            *entry_p = search_dir_p->entrys[i];
            search_status = 1;
            break;
        }
    }
    return search_status;
}

int beanfs_add_entry(struct beanfs_dir_entry *new_entry, struct beanfs_sb_info *sb_info_p,
                     struct beanfs_inode_info *d_inode_p, FILE *v_device)
{
    int status = -1;
    struct beanfs_dir curdir = {0};
    uint32_t dir_addr = UINT32_MAX;
    int8_t entrys_count = 0;
    // for convience, there must 1 block recording entry_information for every dir
    // that is every directory only have limited entry (18 entrys)
    dir_addr = d_inode_p->i_addr.d_addr[0];
    //assert(d_inode_p->i_blocks == 1);
    assert(dir_addr >= sb_info_p->s_first_data_block && dir_addr < sb_info_p->s_blocks_count);
    status = read_block(&curdir, dir_addr, sizeof(struct beanfs_dir), 1, v_device);
    if (status > 0) {
        // begin add entry
        if (curdir.len < MAX_ENTRY) {
            for (entrys_count = 0; entrys_count < curdir.len; entrys_count++) {
                if (strcmp(curdir.entrys[entrys_count].d_name, new_entry->d_name) == 0) {
                    // same name in the entry
                    fprintf(stderr, "FILE %s exists \n", new_entry->d_name);
                    return status;
                }
            }
            curdir.entrys[curdir.len++] = *new_entry;
            status = write2block(&curdir, dir_addr, sizeof(struct beanfs_dir), 1, v_device);
            if (status > 0) {
                status = 1;
            } else {
                status = -1;
            }
        } else {
            fprintf(stderr, "this directory has reached the limitation of entrys \n");
        }
    }
    
    return status;
}

int beanfs_delete_entry(const char fname[], struct beanfs_sb_info *sb_info_p,
                        struct beanfs_inode_info *d_inode_p, struct beanfs_dir_entry *deleted_entry,
                        FILE *v_device)
{
    int status = -1;
    struct beanfs_dir curdir = {0};
    uint32_t dir_addr = UINT32_MAX;
    int8_t entry_count = 0;
    
    dir_addr = d_inode_p->i_addr.d_addr[0];
    status = read_block(&curdir, dir_addr, sizeof(struct beanfs_dir), 1, v_device);
    if (status > 0) {
        // iterator curdir
        for (entry_count = 0; entry_count < curdir.len; entry_count++) {
            if (strcmp(curdir.entrys[entry_count].d_name, fname) == 0) {
                // find the entry to be delete
                *deleted_entry = curdir.entrys[entry_count];
                curdir.len--;
                for (int8_t i = entry_count; i < curdir.len; i++) {
                    curdir.entrys[i] = curdir.entrys[i + 1];
                }
                break;
            }
        }
        if (entry_count < curdir.len) {
            status = 1;
        } else {
            deleted_entry->d_name[0] = '\0';
            deleted_entry->d_ino = UINT32_MAX;
            deleted_entry->d_file_type = '\0';
            fprintf(stderr, "No such file ( %s ) or directory", fname);
        }
    }
    return status;
}

int beanfs_lookup(char full_path[], struct beanfs_sb_info *sb_info_p, struct beanfs_dir_entry *entry_p, FILE *v_device)
{
    int status = -1;
    uint32_t search_ino = 0;
    int search_result = 0;
    char search_fname[MAX_ENTRY_NAME] = {'\0'};
    char *left_sep_pos = NULL;
    char *right_sep_pos = NULL;
    const char *pathsep = "/";
    struct beanfs_inode search_inode;
    struct beanfs_inode_info search_inode_info;
    struct beanfs_dir cur_search_dir;
    struct beanfs_dir_entry tmp_entry = {0, "root", 'd'};
    
    // transform root inode
    left_sep_pos = full_path;
    right_sep_pos = full_path;
    // init root as search path
    // read root dir_info
    beanfs_read_inode(sb_info_p, &search_inode, sb_info_p->s_first_inode_block + search_ino, v_device);
    beanfs_transform2inode_info(&search_inode, &search_inode_info, search_ino);
    read_block(&cur_search_dir, search_inode_info.i_addr.d_addr[0], sizeof(struct beanfs_dir), 1, v_device);
    
    do {
        right_sep_pos = strpbrk(left_sep_pos + 1, pathsep);
        if (right_sep_pos == NULL) {
            strcpy(search_fname, left_sep_pos + 1);
        } else {
            if (left_sep_pos[1] != '\0') {
                strncpy(search_fname, left_sep_pos + 1, right_sep_pos - left_sep_pos - 1);
                search_fname[right_sep_pos - left_sep_pos - 1] = '\0';
                left_sep_pos = right_sep_pos;
            } else {
                search_fname[0] = '\0';
            }
        }
        
        if (search_fname[0] != '\0') {
            search_result = search_dir_entry(search_fname, &cur_search_dir, &tmp_entry);
            if (search_result && tmp_entry.d_file_type == 'd') {
                beanfs_read_inode(sb_info_p, &search_inode, sb_info_p->s_first_inode_block + tmp_entry.d_ino, v_device);
                beanfs_transform2inode_info(&search_inode, &search_inode_info, tmp_entry.d_ino);
                read_block(&cur_search_dir, search_inode_info.i_addr.d_addr[0], sizeof(struct beanfs_dir), 1, v_device);
                status = 1;
            } else {
                if (search_result && right_sep_pos == NULL) {
                    // reached end
                    // find what u want
                    status = 1;
                } else {
                    // can't find the file
                    //fprintf(stderr, " %s is not a valid path", full_path);
                    status = -1;
                }
                break;
            }
            
        } else {
            status = 1;
            break;
        }
    } while (right_sep_pos != NULL);
    
    if (status == 1) {
        *entry_p = tmp_entry;
    } else {
        memset(entry_p, 0, sizeof(struct beanfs_dir_entry));
    }
    return status;
}