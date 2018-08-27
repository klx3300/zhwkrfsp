#ifndef Q_ZHWKRFSP_USTRUCT_H
#define Q_ZHWKRFSP_USTRUCT_H

#include <inttypes.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <dirent.h>

struct Udirent {
    uint64_t d_ino;
    uint64_t d_off;
    uint16_t d_reclen;
    uint8_t d_type;
    char d_name[256];
};

struct Ustat {
    uint64_t st_dev;
    uint64_t st_ino;
    uint32_t st_mode;
    uint64_t st_nlink;
    uint32_t st_uid;
    uint32_t st_gid;
    uint64_t st_rdev;
    uint64_t st_size;
    int64_t st_atim;
    uint64_t st_atimensec;
    int64_t st_mtim;
    uint64_t st_mtimensec;
    int64_t st_ctim;
    uint64_t st_ctimensec;
};

struct Ustatvfs {
    uint64_t f_bsize;
    uint64_t f_frsize;
    uint64_t f_blocks;
    uint64_t f_bfree;
    uint64_t f_bavail;
    uint64_t f_files;
    uint64_t f_ffree;
    uint64_t f_favail;
    uint64_t f_fsid;
    uint64_t f_flag;
    uint64_t f_namemax;
};

void unify_stat(struct Ustat* ustat, struct stat st);
void fall_stat(struct stat* st, struct Ustat ustat);
void unify_statvfs(struct Ustatvfs* uvfs, struct statvfs vfs);
void fall_statvfs(struct statvfs* vfs, struct Ustatvfs uvfs);
void unify_dirent(struct Udirent* udir, struct dirent dir);
void fall_dirent(struct dirent* dir, struct Udirent udir);

#endif