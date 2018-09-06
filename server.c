#define FUSE_USE_VERSION 31
#include "fuse.h"
#include <stdio.h>
#include <string.h>
#include "netwrap.h"
#include "prot.h"
#include <fuse3/fuse.h>
#include "zhwkre/bss.h"
#include "zhwkre/network.h"
#include "zhwkre/concurrent.h"
#include "zhwkre/log.h"

static struct fuse_operations zhwkrfsp = {
    .getattr = fuse_getattr,
    .readlink = fuse_readlink,
    .mkdir = fuse_mkdir,
    .rmdir = fuse_rmdir,
    .symlink = fuse_symlink,
    .chmod = fuse_chmod,
    .truncate = fuse_truncate,
    .open = fuse_open,
    .read = fuse_read,
    .write = fuse_write,
    .statfs = fuse_statfs,
    .release = fuse_release,
    .opendir = fuse_opendir,
    .readdir = fuse_readdir,
    .releasedir = fuse_releasedir,
    .init = fuse_init,
    .access = fuse_access,
    .create = fuse_creat,
    .unlink = fuse_unlink,
    .utimens = fuse_utimens
};

#define OFFSETOF(t, m) ((off_t)&(((t*)(void*)NULL)->m))

static struct options {
    const char* addrxport;
    int show_help;
} options;
#define OPT(t, p) {t, OFFSETOF(struct options, p), 1}
static const struct fuse_opt option_spec[] = {
//    OPT("--addr=%s", addrxport),
    OPT("--help", show_help),
    FUSE_OPT_END
};

int main(int argc, char** argv){
    mu = qMutex_constructor();
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(1, argv);
    options.addrxport = strdup(":10888");
    /*if(fuse_opt_parse(&args, &options, option_spec, NULL) == -1){*/
        /*return 1;*/
    /*}*/
    /*if(options.show_help){*/
        /*printf("%s <addr:port> <mountpoint>\n", argv[0]);*/
        /*return 0;*/
    /*}*/
    // prepare to connect..
    qSocket lisock;
    lisock.domain = qIPv4;
    lisock.type = qStreamSocket;
    lisock.protocol = qDefaultProto;
    if(qSocket_open(lisock)) return -1;
    if(qSocket_bind(lisock, options.addrxport)) return -1;
    if(qStreamSocket_listen(lisock)) return -1;
    qLogInfo("Start accepting incoming connections..");
    sock = qStreamSocket_accept(lisock, ci.srcaddr);
    if(sock.desc == -1){
        return 1;
    }
    // finished setup.
    qSocket_close(lisock);
    qLogInfo("Successfully connected, passing control to fuse_main..");
    struct fuse* f = fuse_new(&args, &zhwkrfsp, sizeof(struct fuse_operations), NULL);
    if(fuse_mount(f, argv[1])){
        fuse_destroy(f);
        return -1;
    }
    if(fuse_loop(f)){
        fuse_unmount(f);
        fuse_destroy(f);
        return -1;
    }
    return 0;
}

