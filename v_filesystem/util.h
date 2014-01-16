//
//  util.h
//  v_filesystem
//
//  Created by WONDER on 12/26/13.
//  Copyright (c) 2013 WONDER. All rights reserved.
//

#ifndef v_filesystem_util_h
#define v_filesystem_util_h

#include "beanfs.h"

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

extern int beanfs_mkfs(FILE *v_device);
extern void beanfs_shell(FILE *v_device);

#endif
