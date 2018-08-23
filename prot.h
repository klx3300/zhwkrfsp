#ifndef Q_ZHWKRFSP_PROT_H
#define Q_ZHWKRFSP_PROT_H

#include <inttypes.h>
#include <sys/stat.h>

#define FILENAME_LEN 4096

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

struct OpOpen {
    char filename[FILENAME_LEN];
    uint8_t flags;
};

struct RpOpen {
    uint64_t file_handle;
    uint8_t ignore_above;
};


#define RD_REGULAR 0
#define RD_STAT    1
#define RD_STATFS  2
#define RD_LINK    3

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

struct OpWrit{
    char filename[FILENAME_LEN];
    uint64_t file_handle;
    uint64_t offset;
    uint64_t count;
    uint8_t write_mode;
};

struct OpClos{
    uint64_t file_handle;
};

#endif