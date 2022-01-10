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
    int fd = yaffs_open(path, O_RDWR, S_IREAD);
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
        yaffs_closedir(dir);
    }
    else return -ENOENT;
}

int yaffs_fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd = yaffs_open(path, O_RDWR, S_IWRITE);
    if (fd < 0) return fd;
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
    int fd = yaffs_open(path,O_CREAT,0);
    yaffs_close(fd);
    if (fd < 0) return -EPERM;
    else return 0;
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
    //.statfs     = yaffs_fuse_statfs,
    .flush      = yaffs_fuse_flush,
    .readdir    = yaffs_fuse_readdir,
    .destroy    = yaffs_fuse_destroy,
    .utimens    = yaffs_fuse_utimens,
};

int main(int argc, char *argv[]) {
    char *back_file = argv[1];
    unsigned long nr_blocks = get_file_size(back_file) / 135168;
    if (nr_blocks == 0) {
        printf("Unable to get file size\n");
        return;
    }
    yaffsfs_OSInitialisation();
    yaffs_nandsim_install_drv("", back_file, nr_blocks, 4, 1);
    yaffs_mount("/");
    printf("tot_size=%lld\n",yaffs_freespace("/"));
    return fuse_main(argc-1,argv+1,&yaffs_fuse_ops,NULL);
}