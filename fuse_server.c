#define FUSE_USE_VERSION 31
#include "fuse.h"
#include "prot.h"
#include "netwrap.h"
#include "zhwkre/log.h"
#include <string.h>
#include "zhwkre/concurrent.h"
#include "error.h"
#include "errno.h"
#include <stdlib.h>
#include <bsd/string.h>
#include <dirent.h>
#include "ustruct.h"

qSocket sock;
qMutex mu;
struct ConnInfo ci;

uint32_t opuid = 0;

#define VPTR(x) ((void*)&x)
#define BSSAPP(bss, st) qbss_append(bss, VPTR(st), sizeof(st))
#define ABRT do{\
    qLogFailfmt("Unrecoverable failure: at %s:%d - %s(). Aborting..", __FILE__, __LINE__, __func__);\
    abort();\
}while(0)
#define WARN(str,rtvl) do{qLogWarnfmt("%s(): %s.", __func__, str);mu.unlock(mu);return rtvl;}while(0)
#define WARNE(str, errn, rtvl) do{qLogWarnfmt("%s(): %s reported error %s.", __func__, str, strerror(errn));mu.unlock(mu);return rtvl;}while(0)
#define SIZEHDR(st) (sizeof(struct OpHdr) + sizeof(st))
#define FNCPY(dst, src) do{char* cpdst = (char*)(dst); const char* cpsrc = (const char*)(src);\
size_t fnsz = strlen(cpsrc); strcpy(cpdst, cpsrc); cpdst[fnsz] = '\0';}while(0)
#define IFFLAG(flg, tst) if((flg) & (tst))

int fuse_getattr(const char* fn, struct stat* st, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): [%s]", __func__, fn);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_READ;
    head.size = sizeof(struct OpHdr) + sizeof(struct OpRead);
    head.ouid = opuid;
    struct OpRead rd;
    size_t fnsz = strlen(fn);
    strcpy(rd.filename, fn);
    rd.filename[fnsz] = '\0';
    rd.read_mode = RD_STAT;
    // send..
    qBinarySafeString sendbuf = qbss_constructor();
    BSSAPP(sendbuf, head);
    BSSAPP(sendbuf, rd);
    if(sockwrite(sock, &sendbuf, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr) + sizeof(struct Ustat)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr) + sizeof(struct Ustat));
        ABRT;
    }
    struct Ustat ustat;
    if(re_read(sock, &ustat, sizeof(struct Ustat), ci)){
        WARN("read reply failed", -EBUSY);
    }
    fall_stat(st, ustat);
    mu.unlock(mu);
    return 0;
}

int fuse_readlink(const char* fn, char* dest, size_t buflen){
    qLogDebugfmt("%s(): [%s] -> char[%lu]\n", __func__, fn, buflen);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_READ;
    head.size = sizeof(struct OpHdr) + sizeof(struct OpRead);
    head.ouid = opuid;
    struct OpRead rd;
    size_t fnsz = strlen(fn);
    strcpy(rd.filename, fn);
    rd.filename[fnsz] = '\0';
    rd.read_mode = RD_LINK;
    qBinarySafeString sendbuf = qbss_constructor();
    BSSAPP(sendbuf, head);
    BSSAPP(sendbuf, rd);
    if(sockwrite(sock, &sendbuf, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    // the filename should be reasonable..
    if(rp.size > sizeof(struct RpHdr) + buflen){
        qLogFailfmt("%s(): protocol error: reply size unreasonable %u\n", __func__, rp.size);
        ABRT;
    }
    if(re_read(sock, dest,rp.size - sizeof(struct RpHdr), ci)){
        WARN("read reply failed", -EBUSY);
    }
    mu.unlock(mu);
    return 0;
}

int fuse_mkdir(const char* dn, mode_t mode){
    char modestr[11];
    strmode(mode, modestr);
    qLogDebugfmt("%s(): [%s] perm %s \n", __func__, dn, modestr);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_OPEN;
    head.size = SIZEHDR(struct OpOpen);
    head.ouid = opuid;
    struct OpOpen op;
    size_t dnsz = strlen(dn);
    strcpy(op.filename, dn);
    op.filename[dnsz] = '\0';
    op.flags = OPEN_MKDIR;
    op.mode = mode;
    // send..
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, op);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return 0;
}

int fuse_rmdir(const char* dn){
    qLogDebugfmt("%s(): [%s]", __func__, dn);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_WRITE;
    head.size = SIZEHDR(struct OpWrit);
    head.ouid = opuid;
    struct OpWrit wr;
    size_t fnsz = strlen(dn);
    strcpy(wr.filename, dn);
    wr.filename[fnsz] = '\0';
    wr.write_mode = WR_RMDIR;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, wr);
    if(sockwrite(sock, &sb, 1)){
        WARN("send failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return 0;
}

int fuse_symlink(const char* fn, const char* srcfn){
    qLogDebugfmt("%s(): %s -> %s", __func__, srcfn, fn);
    mu.lock(mu);
    uint32_t fnsz = strlen(fn), srcfnsz = strlen(srcfn);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_WRITE;
    head.size = SIZEHDR(struct OpWrit) + srcfnsz;
    head.ouid = opuid;
    struct OpWrit wr;
    strcpy(wr.filename, fn);
    wr.filename[fnsz] = '\0';
    char srcbuf[FILENAME_LEN];
    strcpy(srcbuf, srcfn);
    srcbuf[srcfnsz] = '\0';
    wr.write_mode = WR_SYMLINK;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, wr);
    qbss_append(sb, srcbuf, FILENAME_LEN);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return 0;
}

int fuse_chmod(const char* fn, mode_t mode, struct fuse_file_info* fi){
    char modestr[11];
    strmode(mode, modestr);
    qLogDebugfmt("%s(): [%s] perm %s \n", __func__, fn, modestr);
    uint32_t fnsz = strlen(fn);
    struct OpHdr head;
    head.opid = OPER_WRITE;
    head.size = SIZEHDR(struct OpWrit) + sizeof(mode_t);
    head.ouid = opuid;
    struct OpWrit wr;
    wr.write_mode = WR_CHMOD;
    strcpy(wr.filename, fn);
    wr.filename[fnsz] = '\0';
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, wr);
    BSSAPP(sb, mode);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("Client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return 0;
}

int fuse_truncate(const char* fn, off_t pos, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): [%s] trunc at %lu", __func__, fn, pos);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_WRITE;
    head.size = SIZEHDR(struct OpWrit);
    head.ouid = opuid ++;
    struct OpWrit wr;
    FNCPY(wr.filename, fn);
    wr.write_mode = WR_TRUNC;
    wr.offset = pos;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, wr);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("Client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return 0;
}

int fuse_open(const char* fn, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): %s\n", __func__, fn);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_OPEN;
    head.size = SIZEHDR(struct OpOpen);
    head.ouid = opuid;
    struct OpOpen op;
    FNCPY(op.filename, fn);
    op.flags = 0;
    // perform checks
    if(fi->writepage){
        WARNE("write_page", -EINVAL, -EINVAL);
    }
    if(fi->flags & (O_APPEND | O_ASYNC | O_CLOEXEC | O_DSYNC | O_EXCL | O_SYNC)){
        IFFLAG(fi->flags, O_APPEND){
            qLogWarn("APPEND");
        }
        IFFLAG(fi->flags, O_ASYNC){
            qLogWarn("ASYNC");
        }
        IFFLAG(fi->flags, O_CLOEXEC){
            qLogWarn("CLOEXEC");
        }
        IFFLAG(fi->flags, O_DSYNC){
            qLogWarn("DSYNC");
        }
        IFFLAG(fi->flags, O_EXCL){
            qLogWarn("EXCL");
        }
        IFFLAG(fi->flags, O_SYNC){
            qLogWarn("SYNC");
        }
        WARNE("Unsupported flags", -EINVAL, -EINVAL);
    }
    IFFLAG(fi->flags, O_TRUNC){
        op.flags |= OPEN_TRUNC;
    }
    IFFLAG(fi->flags, O_NOFOLLOW){
        op.flags |= OPEN_NOFOLLOW;
    }
    if(fi->direct_io) op.flags |= OPEN_DIRECT;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, op);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("Client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr) + sizeof(struct RpOpen)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr) + sizeof(struct RpOpen));
        ABRT;
    }
    struct RpOpen ro;
    if(re_read(sock, &ro, sizeof(struct RpOpen), ci)){
        WARN("read reply failed", -EBUSY);
    }
    if(!ro.ignore_above){
        fi->fh = ro.file_handle;
        qLogDebugfmt("%s(): file handle set as %lu", __func__, fi->fh);
    }
    mu.unlock(mu);
    return 0;
}

int fuse_read(const char* fn, char* buf, size_t bufsize, off_t pos, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): [%s]@%lu, <%lu> -> char[%lu]", __func__, fn, fi->fh, pos, bufsize);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_READ;
    head.size = SIZEHDR(struct OpRead);
    head.ouid = opuid;
    struct OpRead rd;
    FNCPY(rd.filename, fn);
    rd.file_handle = fi->fh;
    rd.read_mode = RD_REGULAR;
    rd.offset = pos;
    rd.count = bufsize;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, rd);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size > bufsize) {
        qLogFailfmt("%s(): client execution error: too large reply %u > %lu", __func__, rp.rtvl, bufsize);
        ABRT;
    }
    if(rp.size != sizeof(struct RpHdr) + rp.rtvl){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr) + rp.rtvl);
        ABRT;
    }
    if(re_read(sock, buf, rp.rtvl, ci)){
        WARN("read reply failed", -EBUSY);
    }
    mu.unlock(mu);
    return rp.rtvl;
}

int fuse_write(const char* fn, const char* buf, size_t bufsize, off_t pos, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): [%s]@%lu, <%lu> <- char[%lu]", __func__, fn, fi->fh, pos, bufsize);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_WRITE;
    head.size = SIZEHDR(struct OpWrit) + bufsize;
    head.ouid = opuid;
    struct OpWrit wr;
    FNCPY(wr.filename, fn);
    wr.file_handle = fi->fh;
    wr.write_mode = WR_REGULAR;
    wr.offset = pos;
    wr.count = bufsize;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, wr);
    qbss_append(sb, buf, bufsize);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return rp.rtvl;
}

int fuse_statfs(const char* fn, struct statvfs* fstat){
    qLogDebugfmt("%s(): %s (ignored)", __func__, fn);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_READ;
    head.size = sizeof(struct OpHdr) + sizeof(struct OpRead);
    head.ouid = opuid;
    struct OpRead rd;
    size_t fnsz = strlen(fn);
    strcpy(rd.filename, fn);
    rd.filename[fnsz] = '\0';
    rd.read_mode = RD_STATFS;
    // send..
    qBinarySafeString sendbuf = qbss_constructor();
    BSSAPP(sendbuf, head);
    BSSAPP(sendbuf, rd);
    if(sockwrite(sock, &sendbuf, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr) + sizeof(struct Ustatvfs)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr) + sizeof(struct Ustatvfs));
        ABRT;
    }
    struct Ustatvfs uvfs;
    if(re_read(sock, &uvfs, sizeof(struct Ustatvfs), ci)){
        WARN("read reply failed", -EBUSY);
    }
    fall_statvfs(fstat, uvfs);
    mu.unlock(mu);
    return 0;
}

int fuse_release(const char* fn, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): [%s]@%lu", __func__, fn, fi->fh);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_CLOSE;
    head.size = SIZEHDR(struct OpClos);
    head.ouid = opuid;
    struct OpClos cl;
    cl.file_handle = fi->fh;
    cl.is_dir = 0;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, cl);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return 0;
}

int fuse_opendir(const char* dn, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): %s\n", __func__, dn);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_OPEN;
    head.size = SIZEHDR(struct OpOpen);
    head.ouid = opuid;
    struct OpOpen op;
    FNCPY(op.filename, dn);
    op.flags = 0;
    // perform checks
    if(fi->writepage){
        WARNE("write_page", -EINVAL, -EINVAL);
    }
    if(fi->flags & (O_TRUNC | O_APPEND | O_ASYNC | O_CLOEXEC | O_DSYNC | O_EXCL | O_SYNC)){
        IFFLAG(fi->flags, O_APPEND){
            qLogWarn("APPEND");
        }
        IFFLAG(fi->flags, O_ASYNC){
            qLogWarn("ASYNC");
        }
        IFFLAG(fi->flags, O_CLOEXEC){
            qLogWarn("CLOEXEC");
        }
        IFFLAG(fi->flags, O_DSYNC){
            qLogWarn("DSYNC");
        }
        IFFLAG(fi->flags, O_EXCL){
            qLogWarn("EXCL");
        }
        IFFLAG(fi->flags, O_SYNC){
            qLogWarn("SYNC");
        }
        IFFLAG(fi->flags, O_TRUNC){
            qLogWarn("TRUNC");
        }
        WARNE("Unsupported flags", -EINVAL, -EINVAL);
    }
    
    IFFLAG(fi->flags, O_NOFOLLOW){
        op.flags |= OPEN_NOFOLLOW;
    }
    op.flags |= OPEN_DIRECTORY;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, op);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("Client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr) + sizeof(struct RpOpen)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr) + sizeof(struct RpOpen));
        ABRT;
    }
    struct RpOpen ro;
    if(re_read(sock, &ro, sizeof(struct RpOpen), ci)){
        WARN("read reply failed", -EBUSY);
    }
    if(!ro.ignore_above){
        fi->fh = ro.file_handle;
        qLogDebugfmt("%s(): file handle set as %lu", __func__, fi->fh);
    }
    mu.unlock(mu);
    return 0;
}

int fuse_readdir(const char* dn, void* buf, fuse_fill_dir_t fill, off_t offset, struct fuse_file_info* fi, enum fuse_readdir_flags flags){
    qLogDebugfmt("%s(): [%s]@%lu, <%lu> ", __func__, dn, fi->fh, offset);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_READ;
    head.size = SIZEHDR(struct OpRead);
    head.ouid = opuid;
    struct OpRead rd;
    FNCPY(rd.filename, dn);
    rd.file_handle = fi->fh;
    rd.read_mode = RD_DIR;
    rd.offset = offset;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, rd);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size < sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size too small %u < %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    qBinarySafeString repcont = qbss_constructor();
    if(sockread(sock, &repcont, rp.size - sizeof(struct RpHdr))){
        qbss_destructor(repcont);
        WARN("read reply failed", -EBUSY);
    }
    struct Udirent* dirbeg = (struct Udirent*) repcont.str;
    size_t dircnt = (rp.size - sizeof(struct RpHdr)) / sizeof(struct Udirent);
    for(struct Udirent* dirit = dirbeg; dirit < dirbeg + dircnt; dirit++){
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = dirit->d_ino;
        st.st_mode = dirit->d_type << 12;
        if(fill(buf, dirit->d_name, &st, 0, 0)) break;
    }
    qbss_destructor(repcont);
    mu.unlock(mu);
    return 0;
}

int fuse_releasedir(const char* dn, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): [%s]@%lu", __func__, dn, fi->fh);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_CLOSE;
    head.size = SIZEHDR(struct OpClos);
    head.ouid = opuid;
    struct OpClos cl;
    cl.file_handle = fi->fh;
    cl.is_dir = 1;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, cl);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return 0;
}

void* fuse_init(struct fuse_conn_info* conn, struct fuse_config* cfg){
    // for now, after reading MANUAL, i think i can return NULL here.
    return NULL;
}

int fuse_access(const char* fn, int amode){
    char modestr[11];
    strmode(amode, modestr);
    qLogDebugfmt("%s(): [%s] perm %s \n", __func__, fn, modestr);
    uint32_t fnsz = strlen(fn);
    struct OpHdr head;
    head.opid = OPER_WRITE;
    head.size = SIZEHDR(struct OpWrit) + sizeof(mode_t);
    head.ouid = opuid;
    struct OpWrit wr;
    wr.write_mode = WR_ACCES;
    strcpy(wr.filename, fn);
    wr.filename[fnsz] = '\0';
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, wr);
    BSSAPP(sb, amode);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("Client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return 0;
}

int fuse_unlink(const char* fn){
    qLogDebugfmt("%s(): [%s] !", __func__, fn);
    uint32_t fnsz = strlen(fn);
    struct OpHdr head;
    head.opid = OPER_WRITE;
    head.size = SIZEHDR(struct OpWrit);
    head.ouid = opuid;
    struct OpWrit wr;
    wr.write_mode = WR_REMOVE;
    strcpy(wr.filename, fn);
    wr.filename[fnsz] = '\0';
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, wr);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("Client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr));
        ABRT;
    }
    mu.unlock(mu);
    return 0;
}

int fuse_creat(const char* fn, mode_t mode, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): %s\n", __func__, fn);
    mu.lock(mu);
    opuid ++;
    struct OpHdr head;
    head.opid = OPER_OPEN;
    head.size = SIZEHDR(struct OpOpen);
    head.ouid = opuid;
    struct OpOpen op;
    FNCPY(op.filename, fn);
    op.flags = 0;
    op.mode = mode;
    // perform checks
    if(fi->writepage){
        WARNE("write_page", -EINVAL, -EINVAL);
    }
    if(fi->flags & (O_APPEND | O_ASYNC | O_CLOEXEC | O_DSYNC | O_EXCL | O_SYNC)){
        IFFLAG(fi->flags, O_APPEND){
            qLogWarn("APPEND");
        }
        IFFLAG(fi->flags, O_ASYNC){
            qLogWarn("ASYNC");
        }
        IFFLAG(fi->flags, O_CLOEXEC){
            qLogWarn("CLOEXEC");
        }
        IFFLAG(fi->flags, O_DSYNC){
            qLogWarn("DSYNC");
        }
        IFFLAG(fi->flags, O_EXCL){
            qLogWarn("EXCL");
        }
        IFFLAG(fi->flags, O_SYNC){
            qLogWarn("SYNC");
        }
        WARNE("Unsupported flags", -EINVAL, -EINVAL);
    }
    IFFLAG(fi->flags, O_TRUNC){
        op.flags |= OPEN_TRUNC;
    }
    IFFLAG(fi->flags, O_NOFOLLOW){
        op.flags |= OPEN_NOFOLLOW;
    }
    if(fi->direct_io) op.flags |= OPEN_DIRECT;
    op.flags |= OPEN_CREAT;
    qBinarySafeString sb = qbss_constructor();
    BSSAPP(sb, head);
    BSSAPP(sb, op);
    if(sockwrite(sock, &sb, 1)){
        WARN("write failed", -EBUSY);
    }
    struct RpHdr rp;
    if(wait_reply(sock, &rp, ci)){
        WARN("wait reply failed", -EBUSY);
    }
    if(rp.rtvl < 0){
        WARNE("Client", rp.rtvl, rp.rtvl);
    }
    if(rp.size != sizeof(struct RpHdr) + sizeof(struct RpOpen)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr) + sizeof(struct RpOpen));
        ABRT;
    }
    struct RpOpen ro;
    if(re_read(sock, &ro, sizeof(struct RpOpen), ci)){
        WARN("read reply failed", -EBUSY);
    }
    if(!ro.ignore_above){
        fi->fh = ro.file_handle;
        qLogDebugfmt("%s(): file handle set as %lu", __func__, fi->fh);
    }
    mu.unlock(mu);
    return 0;
}

