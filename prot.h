#ifndef Q_ZHWKRFSP_PROT_H
#define Q_ZHWKRFSP_PROT_H

#include <inttypes.h>
#include <sys/stat.h>
#include "zhwkre/bss.h"
#include "zhwkre/network.h"
#include "netwrap.h"

#define FILENAME_LEN 4096

#define OPER_OPEN  0
#define OPER_READ  1
#define OPER_WRITE 2
#define OPER_CLOSE 3

struct OpHdr {
    uint32_t size;
    uint32_t ouid;
    uint32_t opid;
};

struct RpHdr {
    uint32_t size;
    uint32_t ouid;
    int32_t rtvl;
};

#define OPEN_CREAT     1
#define OPEN_DIRECT    2
#define OPEN_DIRECTORY 4
#define OPEN_NOFOLLOW  8
#define OPEN_TRUNC     16
#define OPEN_MKDIR     32

struct OpOpen {
    char filename[FILENAME_LEN];
    uint8_t flags;
    uint16_t mode;
};

struct RpOpen {
    uint64_t file_handle;
    uint8_t ignore_above;
};


#define RD_REGULAR 0
#define RD_STAT    1
#define RD_STATFS  2
#define RD_LINK    3
#define RD_DIR     4

struct OpRead {
    char filename[FILENAME_LEN];
    uint64_t file_handle;
    uint64_t offset;
    uint64_t count;
    uint8_t  read_mode;
};

#define RDDIR_HAVESTAT 1

struct RdDir{
    char filename[FILENAME_LEN];
    uint8_t extflags;
    struct stat filestat;
};

#define WR_REGULAR 0
#define WR_CHMOD   1
#define WR_SYMLINK 2
#define WR_REMOVE  3
#define WR_TRUNC   4
#define WR_ACCES   5
#define WR_RMDIR   6

struct OpWrit{
    char filename[FILENAME_LEN];
    uint64_t file_handle;
    uint64_t offset;
    uint64_t count;
    uint8_t write_mode;
};

struct OpClos{
    uint64_t file_handle;
    uint8_t is_dir;
};

int wait_operation(qSocket streamsock, struct OpHdr* desthdr, struct ConnInfo ci);
int as_open(qSocket streamsock, struct OpOpen* dest, struct ConnInfo ci);
int as_read(qSocket streamsock, struct OpRead* dest, struct ConnInfo ci);
int as_write(qSocket streamsock, struct OpWrit* dest, void* buffer, size_t remain, struct ConnInfo ci);
int as_close(qSocket streamsock, struct OpClos* dest, struct ConnInfo ci);

int wait_reply(qSocket sock, struct RpHdr* rephdr, struct ConnInfo ci);
int re_open(qSocket sock, struct RpOpen* dest, struct ConnInfo ci);
int re_read(qSocket sock, void* buffer, size_t remain, struct ConnInfo ci);
// write and close doesn't return extra informations.

#endif
