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

int fuse_getattr(const char* fn, struct stat* st, struct fuse_file_info* fi){
    qLogDebugfmt("%s(): [%s]@%lu", __func__, fn, fi->fh);
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
    if(rp.size != sizeof(struct RpHdr) + sizeof(struct stat)){
        qLogFailfmt("%s(): protocol error: reply size mismatch %u != %lu", __func__, rp.size, sizeof(struct RpHdr) + sizeof(struct stat));
        ABRT;
    }
    if(re_read(sock, st, sizeof(struct stat), ci)){
        WARN("read reply failed", -EBUSY);
    }
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
    wr.write_mode = WR_REMOVE;
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
    qBinarySafeString sb;
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
    qLogDebugfmt("%s(): [%s]@%lu trunc at %lu", __func__, fn, fi->fh, pos);
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

}

int fuse_read(const char* fn, char* buf, size_t bufsize, off_t pos, struct fuse_file_info* fi){

}

int fuse_write(const char* fn, const char* buf, size_t bufsize, off_t pos, struct fuse_file_info fi){

}

int fuse_statfs(const char* fn, struct statvfs* fstat){

}

int fuse_release(const char* fn, struct fuse_file_info* fi){

}

int fuse_opendir(const char* dn, struct fuse_file_info* fi){

}

int fuse_readdir(const char* dn, void* buf, fuse_fill_dir_t fill, off_t offset, struct fuse_file_info* fi, enum fuse_readdir_flags flags){

}

int fuse_releasedir(const char* dn, struct fuse_file_info* fi){

}

void* fuse_init(struct fuse_conn_info* conn, struct fuse_config* cfg){

}

int fuse_access(const char* fn, int amode){

}

int fuse_creat(const char* fn, mode_t mode, struct fuse_file_info* fi){

}

