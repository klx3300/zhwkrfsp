#include "prot.h"
#include "zhwkre/log.h"
#include "netwrap.h"
#include "zhwkre/bss.h"
#include <error.h>
#include <errno.h>
#include <string.h>

int wait_operation(qSocket streamsock, struct OpHdr* desthdr, struct ConnInfo ci){
    qBinarySafeString recvbuf = qbss_constructor();
    if(sockread(streamsock, &recvbuf, sizeof(struct OpHdr))){
        qLogWarnfmt("wait_operation(): Read fail at %d[%s] %s\n", ci.connuid, ci.srcaddr, strerror(errno));
        qbss_destructor(recvbuf);
        return -1;
    }
    memcpy(desthdr, recvbuf.str, sizeof(struct OpHdr));
    qbss_destructor(recvbuf);
    return 0;
}

int as_open(qSocket streamsock, struct OpOpen* dest, struct ConnInfo ci){
    qBinarySafeString recvbuf = qbss_constructor();
    if(sockread(streamsock, &recvbuf, sizeof(struct OpOpen))){
        qLogWarnfmt("as_open(): Read fail at %d[%s] %s\n", ci.connuid, ci.srcaddr, strerror(errno));
        qbss_destructor(recvbuf);
        return -1;
    }
    memcpy(dest, recvbuf.str, sizeof(struct OpOpen));
    qbss_destructor(recvbuf);
    return 0;
}

int as_read(qSocket streamsock, struct OpRead* dest, struct ConnInfo ci){
    qBinarySafeString recvbuf = qbss_constructor();
    if(sockread(streamsock, &recvbuf, sizeof(struct OpRead))){
        qLogWarnfmt("as_read(): Read fail at %d[%s] %s\n", ci.connuid, ci.srcaddr, strerror(errno));
        qbss_destructor(recvbuf);
        return -1;
    }
    memcpy(dest, recvbuf.str, sizeof(struct OpRead));
    qbss_destructor(recvbuf);
    return 0;
}

int as_write(qSocket streamsock, struct OpWrit* dest, void* buffer, size_t remain, struct ConnInfo ci){
    qBinarySafeString recvbuf = qbss_constructor();
    int extra = remain - sizeof(struct OpWrit);
    if(sockread(streamsock, &recvbuf, sizeof(struct OpWrit))){
        qLogWarnfmt("as_write(): Read [OPER] fail at %d[%s] %s\n", ci.connuid, ci.srcaddr, strerror(errno));
        qbss_destructor(recvbuf);
        return -1;
    }
    memcpy(dest, recvbuf.str, sizeof(struct OpWrit));
    qbss_destructor(recvbuf);
    recvbuf = qbss_constructor();
    if(sockread(streamsock, &recvbuf, extra)){
        qLogWarnfmt("as_write(): Read [CONT] fail at %d[%s] %s\n", ci.connuid, ci.srcaddr, strerror(errno));
        qbss_destructor(recvbuf);
        return -1;
    }
    memcpy(buffer, recvbuf.str, extra);
    qbss_destructor(recvbuf);
    return 0;
}

int as_close(qSocket streamsock, struct OpClos* dest, struct ConnInfo ci){
    qBinarySafeString recvbuf = qbss_constructor();
    if(sockread(streamsock, &recvbuf, sizeof(struct OpClos))){
        qLogWarnfmt("as_close(): Read fail at %d[%s] %s\n", ci.connuid, ci.srcaddr, strerror(errno));
        qbss_destructor(recvbuf);
        return -1;
    }
    memcpy(dest, recvbuf.str, sizeof(struct OpClos));
    qbss_destructor(recvbuf);
    return 0;
}
