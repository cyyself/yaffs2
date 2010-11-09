/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2010 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "test_yaffs_open_EINVAL.h"

static int handle=0;
int test_yaffs_open_EINVAL(void){
	int output=0;
	int error_code=0;
	/*printf("path %s\n",path); */
	handle=yaffs_open(FILE_PATH, 255,FILE_MODE);
	if (handle==-1){
		error_code=yaffs_get_error();
		if (abs(error_code)== EEXIST){	/* yaffs open does not check the mode. 
						so yaffs open does not return EINVAL.
						This causes the error EEXIST to happen instead
						because both O_CREAT and O_EXCL are set */ 
			return 1;
		}
		else {
			printf("different error than expected\n");
			return -1;
		}
	}
	else {
		printf(" file opened with bad mode.(which is a bad thing)\n");
		return -1;
	}
	/* the program should not get here but the compiler is complaining */
	return -1;
}
int test_yaffs_open_EINVAL_clean(void){
	if (handle >=0){
		return yaffs_close(handle);
	}
	else {
		return 1;	/* the file failed to open so there is no need to close it*/
	}
}
