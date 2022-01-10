#define FUSE_USE_VERSION 30

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <fuse.h>
#include <errno.h>
#include <linux/fs.h>
#include "yaffsfs.h"
#include "yaffs_guts.h" /* Only for dumping device innards */
#include "yaffs_endian.h" /*For testing the swap_u64 macro */
#include "yaffs_nandsim_file.h"

int random_seed;
int simulate_power_failure;
struct yaffs_dev *dev;

#ifdef CONFIG_YAFFS_MONITOR_MALLOC
extern int malloc_currently_allocated;
#endif

unsigned long get_file_size(const char *path) {
    int fd = open(path, O_RDWR);
    if (fd < 0) return 0;
    struct stat statbuf;
    int ret = fstat(fd,&statbuf);
    if (ret < 0) return 0;
    size_t disk_size = statbuf.st_size;
    if (disk_size == 0) {
        // check if disk
        int tmp = ioctl(fd,BLKGETSIZE64,&disk_size);
        if (tmp==-1) return 0; // neither file nor block device
    }
    close(fd);
    return disk_size;
}

int yaffs_fuse_getattr(const char *path, struct stat *st) {
#ifdef CONFIG_YAFFS_MONITOR_MALLOC
    if (strcmp(path,"/.yaffs_memory") == 0) {
        st->st_mode = S_IFREG | 400;
        st->st_nlink = 1;
        st->st_size = 0;
        return 0;
    }
#endif
    struct yaffs_stat statbuf;
    int res = yaffs_lstat(path,&statbuf);
    if (res == -1) return -ENOENT;
    st->st_uid     = getuid();
    st->st_gid     = getgid();
    st->st_mode    = statbuf.st_mode;
    st->st_nlink   = statbuf.st_nlink;
    st->st_mtime   = statbuf.st_mode;
    st->st_size    = statbuf.st_size;
    st->st_blksize = statbuf.st_blksize;
    st->st_blocks  = statbuf.st_blocks;
    return res;
}

int yaffs_fuse_read(const char *path, char *buf, size_t nByte, off_t offset, struct fuse_file_info *fi) {
#ifdef CONFIG_YAFFS_MONITOR_MALLOC
    if (strcmp(path,"/.yaffs_memory") == 0) {
        if (offset == 0) {
            sprintf(buf,"yaffs_memory = %d\n",malloc_currently_allocated);
            return strlen(buf);
        }
        else return 0;
    }
#endif
    int fd = yaffs_open(path, O_RDONLY, S_IREAD);
    if (fd < 0) return fd;
    int res = yaffs_pread(fd,buf,nByte,offset);
    yaffs_close(fd);
    return res;
}

int yaffs_fuse_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    yaffs_DIR *dir = yaffs_opendir(path);
    if (dir) {
        struct yaffs_dirent *de;
        while((de = yaffs_readdir(dir)) != NULL) {
            filler(buffer, de->d_name, NULL, 0);
        }
#ifdef CONFIG_YAFFS_MONITOR_MALLOC
        if (strcmp(path,"/") == 0) filler(buffer, ".yaffs_memory", NULL, 0);
#endif
        yaffs_closedir(dir);
    }
    else return -ENOENT;
}

int yaffs_fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd = yaffs_open(path, O_RDWR, S_IREAD | S_IWRITE);
    if (fd < 0) {
        printf("unable to open fd at %s\n",path);
        return fd;
    }
    int res = yaffs_pwrite(fd,buf,size,offset);
    yaffs_close(fd);
    return res;
}

int yaffs_fuse_flush(const char* path, struct fuse_file_info *fi) {
    return 0;
}

void yaffs_fuse_destroy(void *private) {
    yaffs_unmount("/");
}

int yaffs_fuse_utimens(const char* path, const struct timespec * ts) {
    return 0;
}

int yaffs_fuse_chown(const char* path, uid_t uid, gid_t gid) {
    return 0;
}

int yaffs_fuse_mknod(const char *path, mode_t mode, dev_t rdev) {
    int fd = yaffs_open(path, O_CREAT, S_IREAD | S_IWRITE);
    yaffs_close(fd);
    if (fd < 0) return -EPERM;
    else return 0;
}

int yaffs_fuse_statfs(const char* path, struct statvfs *st) {
    struct yaffs_stat buf;
    yaffs_stat(path,&buf);
    st->f_bsize = 2048;
    st->f_frsize = 2048;
	uint64_t bytes_in_dev =
		    ((uint64_t)
		     ((dev->param.end_block - dev->param.start_block +
		       1))) * ((uint64_t) (dev->param.chunks_per_block *
					   dev->data_bytes_per_chunk));
    st->f_blocks = bytes_in_dev / st->f_bsize;
    uint64_t bytes_free = ((uint64_t) (yaffs_get_n_free_chunks(dev))) *
		    ((uint64_t) (dev->data_bytes_per_chunk));
    st->f_bfree = bytes_free / st->f_bsize;
    st->f_bavail = st->f_bfree;
	st->f_files = 1024;
	st->f_ffree = 512;
    st->f_flag = ST_NOSUID;
    st->f_namemax = 255;
    return 0;
}



struct fuse_operations yaffs_fuse_ops = {
    .getattr    = yaffs_fuse_getattr,
    .mknod      = yaffs_fuse_mknod,
    .mkdir      = yaffs_mkdir,
    .unlink     = yaffs_unlink,
    .rmdir      = yaffs_rmdir,
    .rename     = yaffs_rename,
    .chmod      = yaffs_chmod,
    .chown      = yaffs_fuse_chown,
    .truncate   = yaffs_truncate,
    .read       = yaffs_fuse_read,
    .write      = yaffs_fuse_write,
    .statfs     = yaffs_fuse_statfs,
    .flush      = yaffs_fuse_flush,
    .readdir    = yaffs_fuse_readdir,
    .destroy    = yaffs_fuse_destroy,
    .utimens    = yaffs_fuse_utimens,
};

extern unsigned int yaffs_trace_mask;

int main(int argc, char *argv[]) {
    char *back_file = argv[1];
    unsigned long nr_blocks = get_file_size(back_file) / 135168;
    if (nr_blocks == 0) {
        printf("Unable to get file size\n");
        return;
    }
    yaffs_trace_mask = 0;
    yaffsfs_OSInitialisation();
    dev = yaffs_nandsim_install_drv("", back_file, nr_blocks, 4, 1);
    yaffs_mount("/");
    printf("Mounted!\n");
    return fuse_main(argc-1,argv+1,&yaffs_fuse_ops,NULL);
}