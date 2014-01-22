//
//  commands.h
//  v_filesystem
//
//  Created by WONDER on 1/15/14.
//  Copyright (c) 2014 WONDER. All rights reserved.
//

#ifndef v_filesystem_commands_h
#define v_filesystem_commands_h

extern int beanfs_pwd(struct envrioment_variable *envvars_p);
extern int beanfs_cd(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device);
extern int beanfs_ls(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device);
extern int beanfs_mkdir(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device);
extern int beanfs_rmdir(struct envrioment_variable *envvars_p, struct beanfs_sb_info *sb_info_p, FILE *v_device);
extern int beanfs_clear(void);

#endif
