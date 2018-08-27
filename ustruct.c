#include "ustruct.h"
#include <stdlib.h>
#include <string.h>

void unify_stat(struct Ustat* ustat, struct stat st){
    ustat->st_dev = st.st_dev;
    ustat->st_ino = st.st_ino;
    ustat->st_mode = st.st_mode;
    ustat->st_nlink = st.st_nlink;
    ustat->st_uid = st.st_uid;
    ustat->st_gid = st.st_gid;
    ustat->st_rdev = st.st_rdev;
    ustat->st_size = st.st_size;
    ustat->st_atim = st.st_atim.tv_sec;
    ustat->st_atimensec = st.st_atim.tv_nsec;
    ustat->st_mtim = st.st_mtim.tv_sec;
    ustat->st_mtimensec = st.st_mtim.tv_nsec;
    ustat->st_ctim = st.st_ctim.tv_sec;
    ustat->st_ctimensec = st.st_ctim.tv_nsec;
}

void fall_stat(struct stat* st, struct Ustat ustat){
    st->st_dev = ustat.st_dev;
    st->st_ino = ustat.st_ino;
    st->st_mode = ustat.st_mode;
    st->st_nlink = ustat.st_nlink;
    st->st_uid = ustat.st_uid;
    st->st_gid = ustat.st_gid;
    st->st_rdev = ustat.st_rdev;
    st->st_size = ustat.st_size;
    st->st_atim.tv_sec = ustat.st_atim;
    st->st_atim.tv_nsec = ustat.st_atimensec;
    st->st_mtim.tv_sec = ustat.st_mtim;
    st->st_mtim.tv_nsec = ustat.st_mtimensec;
    st->st_ctim.tv_sec = ustat.st_ctim;
    st->st_ctim.tv_nsec = ustat.st_ctimensec;
}

void unify_statvfs(struct Ustatvfs* uvfs, struct statvfs vfs){
    uvfs->f_bsize = vfs.f_bsize;
    uvfs->f_frsize = vfs.f_frsize;
    uvfs->f_blocks = vfs.f_blocks;
    uvfs->f_bfree = vfs.f_bfree;
    uvfs->f_bavail = vfs.f_bavail;
    uvfs->f_files = vfs.f_files;
    uvfs->f_ffree = vfs.f_ffree;
    uvfs->f_favail = vfs.f_favail;
    uvfs->f_fsid = vfs.f_fsid;
    uvfs->f_flag = vfs.f_flag;
    uvfs->f_namemax = vfs.f_namemax;
}

void fall_statvfs(struct statvfs* vfs, struct Ustatvfs uvfs){
    vfs->f_bsize = uvfs.f_bsize;
    vfs->f_frsize = uvfs.f_frsize;
    vfs->f_blocks = uvfs.f_blocks;
    vfs->f_bfree = uvfs.f_bfree;
    vfs->f_bavail = uvfs.f_bavail;
    vfs->f_files = uvfs.f_files;
    vfs->f_ffree = uvfs.f_ffree;
    vfs->f_favail = uvfs.f_favail;
    vfs->f_fsid = uvfs.f_fsid;
    vfs->f_flag = uvfs.f_flag;
    vfs->f_namemax = uvfs.f_namemax;
}

void unify_dirent(struct Udirent* udir, struct dirent dir){
    udir->d_ino = dir.d_ino;
    udir->d_off = dir.d_off;
    udir->d_reclen = dir.d_reclen;
    udir->d_type = dir.d_type;
    memcpy(udir->d_name, dir.d_name, 256);
}

void fall_dirent(struct dirent* dir, struct Udirent udir){
    dir->d_ino = udir.d_ino;
    dir->d_off = udir.d_off;
    dir->d_reclen = udir.d_reclen;
    dir->d_type = udir.d_type;
    memcpy(dir->d_name, udir.d_name, 256);
}
