# zhwkrfsp
Remote File System Protocol. This part offers a client-side FUSE filesystem for Linux.

`libfuse` version 3 used due to poor condition of `man 4 fuse`, they even didn't have `FUSE_WRITE`  documented.

### USAGE

#### Compilation

You should first have `fuse3` installed.

In Archlinux, you may perform `pacman -S fuse3`.

Then, compile with `make`. It will directly generate output executable as `server`.

#### Use

This side should run first. The listening port is `10888`, will become an editable parameter in future versions as soon as I figure out how `libfuse` perform the parameter resolution.

To mount, run:

```shell	
$ ./server <mount-point>
```

To unmount, system-provided `umount` will do the job.

### Known Bugs

- `touch` will report error as modification of `a/m/c time` is not currently supported. Will gain this ability in the next version.

---

## Protocol

Note I'm not going to make this a full-feature file system interface. In fact, the poor condition of Linux FUSE man page is making me hard to implement further features.

This is only going to implement a few interfaces, such as `open`,`creat`, `read`, `write`, `close`, and `attr`, with some simple operations formed by combining these.

And the possible file types are also limited. This interface supports **ONLY** regular files, directories and soft links. Soft links are optional, because there are always a way to treat them as regular files..

Permissions is supported, but users is totally not applicable. This means, unless specially specified, the remote permission is totally dependent on the user executed the program.

The protocol is specified in C format. Note this is not encrypted, you can use some proxy program to help you ensure security; this program itself is designed with assumption of working at LAN environment or even `loopback`!

Generally, the protocol contains 4 different operations: `open`, `read`, `write` and `close`. Operation on soft links are implemented by special flags on `open` and `write`, file attributes operations are implemented by special flags on `read` and `write`.

Every operation has a unified header, that is:

```C
struct OpHdr {
    uint32_t size; // include the header and content.
    uint32_t ouid; // Operation Unique ID. This allows better logging.
    uint32_t opid; // Which operation.
};
```

For replies, there are also a unified header, that is:

```C
struct RpHdr {
    uint32_t size; // I don't need to explain twice, do I?
    uint32_t ouid; // logging.
     int32_t rtvl; // Return value. for errors, use negative numbers.
};
```

### Detailed Operations

#### Open(0)

Request:

```C
struct OpOpen {
    char filename[4096]; // GOOD!
    uint8_t flags; // from the lowest bit to highest bit, corresponding to following flags.
    /*
       bool creation; // indicates that if this is a creation request. (1)
       bool direct; // If set, minimize the caching. (2)
       bool directory; // fail if not a directory (4)
       bool nofollow; // fail with ELOOP if is a symbolic link (8)
       bool truncate; // clear the whole file if previously exists. (16)
       bool mkdir; // if this set, will make directory. No file_handle returned.(32)
    */
    uint16_t mode; // for CREAT/MKDIR
};
```

##### Client Side Implementation Notice:

For libfuse:

- if `fuse_file_info->writepage` is set, will return `EINVAL` to kernel directly instead of sending an Open request.

For Open flags:

- `O_APPEND`, `O_ASYNC`, `O_CLOEXEC`, `O_DSYNC`, `O_EXCL`, `O_PATH`, `O_SYNC`, `O_TMPFILE` will cause a `EINVAL` before reach network.
- `O_NOATIME`,  `O_NOCTTY`, `O_NONBLOCK/O_NDELAY` will be ignored.

Reply:

```C
struct RpOpen{
    uint64_t file_handle; // Like descriptor, to let you handle things better.
    uint8_t  ignore_above; // if set, this implies you are doing stateless FS. file handle will not work.
};
```

Very simple.

for mode `mkdir` , you need not to reply.

#### Read(1)

Request:

```C
struct OpRead{
    char filename[4096]; // you are not using file handles, use this
    uint64_t file_handle; // or you are using it, use this
    uint64_t offset; // starting from where
    uint64_t count; // read how much
    uint8_t  read_mode; // 0 REGULAR; 1 STAT; 2 STATFS; 3 RDLINK; 4 RDDIR
};
```

Reply:

No format, just follow by exact the bytes you read. - for regular reads;

when executing read on a directory, the return value should be an array of `struct dirent`.

or

`struct stat` exactly - for stat reads; please always use filename: file handle may be inapplicable.

or

`struct statvfs` exactly - for statfs reads;

or

the link destination filename - for readlink reads;

When you do readlink, the `filename` should be set correctly, and `file_handle` will be ignored in this mode. That's saying, you don't have to `open` it.

Structures mentioned above need special cares; read __Unified Structures__ section below.

For the last 3 cases, just set `rtvl` to 0 if it is succeed.

#### Write(2)

Request:

```C
struct OpWrit{
    char filename[4096];
    uint64_t file_handle;
    uint64_t offset;
    uint64_t count;
    uint8_t write_mode; // 0 REGULAR; 1 CHMOD; 2 SYMLINK; 3 REMOVE; 4 TRUNC; 5 ACCES; 6 RMDIR
};
```

followed by:

content need to be written - for regular writes

or

`mode_t` - for chmod/acces, no open needed also.

acces mode will test the permission against the specified file.

or

the filename of the symbolic link source file/the file you want to delete, filled to 4096 bytes - for symlink and remove

when you cfreating symbolic links, directly send write request with `filename` set to the link name, no open needed, as `file_handle` will get ignored if mode is `SYMLINK/REMOVE`.

or

nothing - for truncate files.

in this case, the count will be ignored, and the file will be truncated at position `offset`.

Reply:

No reply except the unified header.

#### Close(3)

Request:

```C
struct OpClos{
	uint64_t file_handle;
    uint8_t is_dir;
};
```

Reply:

No reply except the unified header.



## Unified Structures

`struct dirent`, `struct stat` and `struct statvfs` is very platform-dependent.

So It's necessary to define some unified form of them.

first, we have

```C
struct dirent{
    ino_t d_ino;
    off_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];
};
```

in a typical Linux-x86_64, we have

```C
typedef ino_t unsigned long int;
typedef off_t unsigned long int;
typedef size_t unsigned long int;
typedef dev_t unsigned long int;
```

then

```C
struct Udirent{
    unsigned long int d_ino;
    unsigned long int d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];
};
```

second, we have

```C
// there are very much macro tests.
struct stat
  {
    __dev_t st_dev;		/* Device.  */
#ifndef __x86_64__
    unsigned short int __pad1;
#endif
#if defined __x86_64__ || !defined __USE_FILE_OFFSET64
    __ino_t st_ino;		/* File serial number.	*/
#else
    __ino_t __st_ino;			/* 32bit file serial number.	*/
#endif
#ifndef __x86_64__
    __mode_t st_mode;			/* File mode.  */
    __nlink_t st_nlink;			/* Link count.  */
#else
    __nlink_t st_nlink;		/* Link count.  */
    __mode_t st_mode;		/* File mode.  */
#endif
    __uid_t st_uid;		/* User ID of the file's owner.	*/
    __gid_t st_gid;		/* Group ID of the file's group.*/
#ifdef __x86_64__
    int __pad0;
#endif
    __dev_t st_rdev;		/* Device number, if device.  */
#ifndef __x86_64__
    unsigned short int __pad2;
#endif
#if defined __x86_64__ || !defined __USE_FILE_OFFSET64
    __off_t st_size;			/* Size of file, in bytes.  */
#else
    __off64_t st_size;			/* Size of file, in bytes.  */
#endif
    __blksize_t st_blksize;	/* Optimal block size for I/O.  */
#if defined __x86_64__  || !defined __USE_FILE_OFFSET64
    __blkcnt_t st_blocks;		/* Number 512-byte blocks allocated. */
#else
    __blkcnt64_t st_blocks;		/* Number 512-byte blocks allocated. */
#endif
#ifdef __USE_XOPEN2K8
    /* Nanosecond resolution timestamps are stored in a format
       equivalent to 'struct timespec'.  This is the type used
       whenever possible but the Unix namespace rules do not allow the
       identifier 'timespec' to appear in the <sys/stat.h> header.
       Therefore we have to handle the use of this header in strictly
       standard-compliant sources special.  */
    struct timespec st_atim;		/* Time of last access.  */
    struct timespec st_mtim;		/* Time of last modification.  */
    struct timespec st_ctim;		/* Time of last status change.  */
# define st_atime st_atim.tv_sec	/* Backward compatibility.  */
# define st_mtime st_mtim.tv_sec
# define st_ctime st_ctim.tv_sec
#else
    __time_t st_atime;			/* Time of last access.  */
    __syscall_ulong_t st_atimensec;	/* Nscecs of last access.  */
    __time_t st_mtime;			/* Time of last modification.  */
    __syscall_ulong_t st_mtimensec;	/* Nsecs of last modification.  */
    __time_t st_ctime;			/* Time of last status change.  */
    __syscall_ulong_t st_ctimensec;	/* Nsecs of last status change.  */
#endif
#ifdef __x86_64__
    __syscall_slong_t __glibc_reserved[3];
#else
# ifndef __USE_FILE_OFFSET64
    unsigned long int __glibc_reserved4;
    unsigned long int __glibc_reserved5;
# else
    __ino64_t st_ino;			/* File serial number.	*/
# endif
#endif
  };
```

It's not hard to extract necessary stuff..

```C
struct Ustat{
    unsigned long int st_dev;
    unsigned long int st_ino;
    uint32_t st_mode;
    unsigned long int st_nlink;
    uint32_t st_uid;
    uint32_t st_gid;
    unsigned long int st_rdev;
    unsigned long int st_size;
    // block related information not necessary, so ignored
    long int st_atime;
    unsigned long int st_atimensec;
    long int st_mtime;
    unsigned long int st_mtimensec;
    long int st_ctime;
    unsigned long int st_ctimensec;
};
```

and..

```C
struct statvfs
  {
    unsigned long int f_bsize;
    unsigned long int f_frsize;
#ifndef __USE_FILE_OFFSET64
    __fsblkcnt_t f_blocks;
    __fsblkcnt_t f_bfree;
    __fsblkcnt_t f_bavail;
    __fsfilcnt_t f_files;
    __fsfilcnt_t f_ffree;
    __fsfilcnt_t f_favail;
#else
    __fsblkcnt64_t f_blocks;
    __fsblkcnt64_t f_bfree;
    __fsblkcnt64_t f_bavail;
    __fsfilcnt64_t f_files;
    __fsfilcnt64_t f_ffree;
    __fsfilcnt64_t f_favail;
#endif
    unsigned long int f_fsid;
#ifdef _STATVFSBUF_F_UNUSED
    int __f_unused;
#endif
    unsigned long int f_flag;
    unsigned long int f_namemax;
    int __f_spare[6];
  };
```

to

```C
struct Ustatvfs {
    unsigned long int f_bsize;
    unsigned long int f_frsize;
    unsigned long int f_blocks;
    unsigned long int f_bfree;
    unsigned long int f_bavail;
    unsigned long int f_files;
    unsigned long int f_ffree;
    unsigned long int f_favail;
    unsigned long int f_fsid;
    unsigned long int f_flag;
    unsigned long int f_namemax;
};
```

