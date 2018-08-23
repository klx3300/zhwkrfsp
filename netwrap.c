#include "netwrap.h"
#include "zhwkre/log.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int listenat(const char* addrxport, qSocket* listensock){
    listensock->type = qStreamSocket;
    listensock->domain = qIPv4;
    listensock->protocol = qDefaultProto;
    if(qSocket__open(listensock)){
        qLogFailfmt("Unable to open listening socket: %s\n", strerror(errno));
        return -1;
    }
    if(qSocket_bind(*listensock, addrxport)){
        qLogFailfmt("Unable to bind listening socket: %s\n", strerror(errno));
        return -1;
    }
    if(qStreamSocket_listen(*listensock)){
        qLogFailfmt("Unable to listen: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int serveat(qSocket* listener, void* (*routine)(void* conninfo)){
    qSocket incoming;
    int buid = 0;
    while(1){
        buid ++;
        struct ConnInfo* conn = malloc(sizeof(struct ConnInfo));
        incoming = qStreamSocket_accept(*listener, conn->srcaddr);
        if(incoming.desc == -1){
            free(conn);
            qLogFailfmt("Unable to serve: %s\n", strerror(errno));
            return -1;
        }
        conn->connuid = buid;
        qRun(routine, conn);// goodbye!
    }
}

int sockread(qSocket sock, qBinarySafeString* bss, int intended){
    int remain = intended;
    char buf[intended];
    while(remain){
        int actr = qStreamSocket_read(sock, buf, remain);
        if(actr == -1){
            qLogFailfmt("Unable to read as intended: %s\n", strerror(errno));
            return -1;
        }
        q__bss_append(bss, buf, actr);
        remain = remain - actr;
    }
    return 0;
}

int sockwrite(qSocket sock, qBinarySafeString* bss, int auto_free){
    int already = 0;
    while(bss->size - already){
        int actw = qStreamSocket_write(sock, bss->str + already, bss->size - already);
        if(actw == -1){
            qLogFailfmt("Unable to write as intended: %s\n", strerror(errno));
            if(auto_free) qbss_destructor(*bss);
            return -1;
        }
        already += actw;
    }
    return 0;
}