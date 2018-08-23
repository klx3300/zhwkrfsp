#ifndef Q_ZHWKRFSP_NETWRAP_H
#define Q_ZHWKRFSP_NETWRAP_H

#include "zhwkre/bss.h"
#include "zhwkre/network.h"
#include "zhwkre/concurrent.h"

struct ConnInfo {
    char srcaddr[40];
    int connuid;
};

int listenat(const char* addrxport, qSocket* listensock);
int serveat(qSocket* listener, void* (*routine)(void* conninfo));
// read will utilize qbss_append(), this is not filling!
int sockread(qSocket sock, qBinarySafeString* bss, int intended);
int sockwrite(qSocket sock, qBinarySafeString* bss, int auto_free);

#endif