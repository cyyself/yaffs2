/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system. 
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __handle_test_h__
#define __handle_test_h__

#include <stdio.h>
#include "yaffsfs.h"

#define YAFFS_MOUNT_POINT "/yflash2/"
#define FILE_PATH "/yflash2/foo"

int open_close_handle_test(int num_of_tests);
int open_handle(void);
void get_error(void);
int dup_test(void);

#endif
