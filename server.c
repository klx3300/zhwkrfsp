#include "fuse.h"
#include <stdio.h>
#include <string.h>
#include "netwrap.h"
#include "prot.h"
#include <fuse3/fuse.h>
#include "zhwkre/bss.h"
#include "zhwkre/network.h"
#include "zhwkre/concurrent.h"

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
    .unlink = fuse_unlink
};

#define OFFSETOF(t, m) ((off_t)&(((t*)(void*)NULL)->m))

static struct options {
    const char* addrxport;
    int show_help;
} options;
#define OPT(t, p) {t, OFFSETOF(struct options, p), 1}
static const struct fuse_opt option_spec[] = {
    OPT("--addr=%s", addrxport),
    OPT("--help", show_help),
    FUSE_OPT_END
};

int main(int argc, char** argv){
    mu = qMutex_constructor();
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    options.addrxport = strdup("127.0.0.1:9999");
    if(fuse_opt_parse(&args, &options, option_spec, NULL) == -1){
        return 1;
    }
    if(options.show_help){
        printf("%s <addr:port> <mountpoint>\n", argv[0]);
        return 0;
    }
    // prepare to connect..
    qSocket lisock;
    if(listenat(options.addrxport, &lisock)){
        return 1;
    }
    sock = qStreamSocket_accept(lisock, ci.srcaddr);
    if(sock.desc == -1){
        return 1;
    }
    // finished setup.
    return fuse_main(args.argc, args.argv, &zhwkrfsp, NULL);
}

