#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdio.h>
#include <fcntl.h>  // O_RDONLY
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h> // strcpy(), strncpy()
#include <sys/stat.h> // struct stat

typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_super_block SUPER;
typedef struct ext2_dir_entry_2 DIR;

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

#define BLOCK_SIZE        1024
#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK+SIZE/sizeof(INODE))

// Block number of EXT2 FS on FD
#define SUPERBLOCK      1
#define GDBLOCK         2
#define BBITMAP         3
#define IBITMAP         4
#define INODEBLOCK      5
#define ROOT_INODE      2

// Default dir and regular file mode
#define D_DIR_MODE      0x41FF // XXX I think KC spoofed us here? -Cameron
#define MASK_MODE       0xF000
#define DIR_MODE        0x4000
#define D_FILE_MODE     0100644 // XXX I think KC spoofed us here? -Cameron
#define FILE_MODE       0x8000
#define SUPER_MAGIC     0xEF53
#define SUPER_USER      0

// Proc status
#define FREE            0
#define READY           1
#define RUNNING         2

// Table sizes
#define NMINODES        100
#define NMOUNT          10
#define NPROC           10
#define NFD             10
#define NOFT            100


// Open File Table
typedef struct oft
{
    int mode;
    int refCount;
    struct minode *inodeptr;
    int offset;
} OFT;

// PROC structure
typedef struct proc
{
    int uid;
    int pid;
    int gid;
    int ppid;
    struct proc* parent;
    int status;
    struct minode* cwd;
    OFT* fd[NFD];
} PROC;

// In-memory inodes structure
typedef struct minode
{
    INODE INODE;    // disk inode
    int dev, ino;
    int refCount;
    int dirty;
    int mounted;
    struct mount* mountptr;
} MINODE;

// In-memory inodes structure
typedef struct mount
{
    int ninodes;
    int nblocks;
    int dev;   // busy DOUBLE CHECK
    MINODE* mounted_inode;
    char name[256];
    char mount_name[64];
} MOUNT;

#endif
