# zhwkrfsp
Remote File System Protocol. This part offers a client-side FUSE filesystem for Linux.

`libfuse` version 3 used due to poor condition of `man 4 fuse`, they even didn't have `FUSE_WRITE`  documented.

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
    uint32_t ouid; // Operation Unique ID. This allows out-of-order execution.
    uint32_t opid; // Which operation.
};
```

For replies, there are also a unified header, that is:

```C
struct RpHdr {
    uint32_t size; // I don't need to explain twice, do I?
    uint32_t ouid; // Which operation you are replying to?
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
    */
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

#### Read(1)

Request:

```C
struct OpRead{
    char filename[4096]; // you are not using file handles, use this
    uint64_t file_handle; // or you are using it, use this
    uint64_t offset; // starting from where
    uint64_t count; // read how much
    uint8_t  read_mode; // 0 REGULAR; 1 STAT; 2 STATFS; 3 RDLINK
};
```

Reply:

No format, just follow by exact the bytes you read. - for regular reads;

when executing read on a directory, the return value should be

```C
struct RdDir{
    char filename[4096];
    uint8_t extflags; // from the lowest bit to highest bit, corresponding to following flags.
    /*
       bool have_stat; // indicates that if this is a creation request. (1)
    */
    struct stat filestat; // only work if have_stat is set.
};
```

or

`struct stat` exactly - for stat reads;

or

`struct statvfs` exactly - for statfs reads;

or

the link destination filename - for readlink reads;

When you do readlink, the `filename` should be set correctly, and `file_handle` will be ignored in this mode. That's saying, you don't have to `open` it.

For the last 3 cases, just set `rtvl` to 0 if it is succeed.

#### Write(2)

Request:

```C
struct OpWrit{
    char filename[4096];
    uint64_t file_handle;
    uint64_t offset;
    uint64_t count;
    uint8_t write_mode; // 0 REGULAR; 1 CHMOD; 2 SYMLINK
};
```

followed by:

content need to be written - for regular writes

or

`mode_t` - for chmod

or

the filename of the symbolic link source file - for symlink

when you creating symbolic links, directly send write request with `filename` set to the link name, no open needed, as `file_handle` will get ignored if mode is `SYMLINK`.

Reply:

No reply except the unified header.

#### Close(3)

Request:

```C
struct OpClos{
	uint64_t file_handle;
};
```

Reply:

No reply except the unified header.