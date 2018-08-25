#ifndef Q_ZHWKRFSP_FUSE_H
#define Q_ZHWKRFSP_FUSE_H

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fuse3/fuse.h>
#include "zhwkre/network.h"

int fuse_getattr(const char* fn, struct stat* st, struct fuse_file_info* fi);
int fuse_readlink(const char* fn, char* dest, size_t buflen);
int fuse_mkdir(const char* dn, mode_t mode);
int fuse_rmdir(const char* dn);
int fuse_symlink(const char* fn, const char* srcfn);
int fuse_chmod(const char* fn, mode_t mode, struct fuse_file_info* fi);
int fuse_truncate(const char* fn, off_t pos, struct fuse_file_info* fi);
int fuse_open(const char* fn, struct fuse_file_info* fi);
int fuse_read(const char* fn, char* buf, size_t bufsize, off_t pos, struct fuse_file_info* fi);
int fuse_write(const char* fn, const char* buf, size_t bufsize, off_t pos, struct fuse_file_info* fi);
int fuse_statfs(const char* fn, struct statvfs* fstat);
int fuse_release(const char* fn, struct fuse_file_info* fi);
int fuse_opendir(const char* dn, struct fuse_file_info* fi);
int fuse_readdir(const char* dn, void* buf, fuse_fill_dir_t fill, off_t offset, struct fuse_file_info* fi, enum fuse_readdir_flags flags);
int fuse_releasedir(const char* dn, struct fuse_file_info* fi);
void* fuse_init(struct fuse_conn_info* conn, struct fuse_config* cfg);
int fuse_access(const char* fn, int amode);
int fuse_creat(const char* fn, mode_t mode, struct fuse_file_info* fi); // create and open

extern qSocket sock;

#endif