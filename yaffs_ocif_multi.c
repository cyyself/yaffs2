/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "yportenv.h"

#include "yaffs_ocif.h"

#include "linux/types.h"
#include "linux/time.h"

#include "linux/kernel.h"
#include "linux/version.h"

#include "uapi/linux/major.h"

#include "yaffs_trace.h"
#include "yaffs_guts.h"
#include "yaffs_linux.h"

//Fix me
static int yaffs_oc_write(struct yaffs_dev *dev, int nand_chunk,
                          const u8 *data, int data_len,
                          const u8 *oob, int oob_len)
{

    return 0;
}

//Fix me
static int yaffs_oc_read(struct yaffs_dev *dev, int nand_chunk,
                         u8 *data, int data_len,
                         u8 *oob, int oob_len,
                         enum yaffs_ecc_result *ecc_result)
{
    return 0;
}

static int yaffs_oc_erase(struct yaffs_dev *dev, int block_no)
{
    return 0;
}

static int yaffs_oc_mark_bad(struct yaffs_dev *dev, int block_no)
{
    return YAFFS_OK;
}

static int yaffs_oc_check_bad(struct yaffs_dev *dev, int block_no)
{
    return YAFFS_OK;
}

static int yaffs_oc_initialise(struct yaffs_dev *dev)
{
	return YAFFS_OK;
}

static int yaffs_oc_deinitialise(struct yaffs_dev *dev)
{
	return YAFFS_OK;
}

void yaffs_oc_drv_install(struct yaffs_dev *dev)
{
    struct yaffs_driver *drv=&dev->drv;

    drv->drv_write_chunk_fn=yaffs_oc_write;
    drv->drv_read_chunk_fn=yaffs_oc_read;
    drv->drv_erase_fn=yaffs_oc_erase;
    drv->drv_mark_bad_fn=yaffs_oc_mark_bad;
    drv->drv_check_bad_fn=yaffs_oc_check_bad;
    drv->drv_initialise_fn=yaffs_oc_initialise;
    drv->drv_deinitialise_fn=yaffs_oc_deinitialise;

    //sth else to do

}