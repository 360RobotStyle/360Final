#include <stdio.h>
#include <stdlib.h> // exit(1)
#include <fcntl.h>  // O_RDONLY
#include <ext2fs/ext2_fs.h>
#include <string.h> // strcpy(), strncpy()
#include <sys/stat.h> // struct stat
#include <time.h>

#define BLKSIZE 1024
#define MSIZE   100
#define BAD     0
#define GOOD    1

typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_super_block SUPER;
typedef struct ext2_dir_entry_2 DIR;
typedef unsigned int u32;


typedef struct minode
{
    INODE inode;
    int dev, ino;
    int refCount;
    int dirty;
} MINODE;

typedef struct proc
{
    struct proc* nextProcPtr;
    int pid;
    int uid;      // uid = 0 for SUPERUSER; non-zero for ordinary user
    MINODE* cwd;  // CWD pointer -> CWD INODE in memory
    // other fields later
} PROC;

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;

INODE myinode;

PROC proc[2];
PROC* running;
PROC* readyQueue;

MINODE minode[MSIZE];
MINODE* root;

u32   InodesBeginBlock;
u32   BlocksCount;
u32   InodesCount;

int fd;
char* device = "mydisk";

// Initialize data structures of LEVEL-1
void init ()
{
    int i;

    proc[0].uid = 0;
    proc[0].cwd = 0;

    proc[1].uid = 1;
    proc[1].cwd = 0;

    running = &proc[0];
    readyQueue = &proc[1];

    for (i = 0; i < 100; i++) minode[i].refCount = 0;

    root = 0;
}


int get_block(int fd, int blk, char* buf)
{
    lseek(fd, (long)(blk*BLKSIZE), 0);
    read(fd, buf, BLKSIZE);
}


/* Super block */
int super()
{
    char buf[BLKSIZE];
    char tmpDate[30];

    // read SUPER block at byte offset 1024
    get_block(fd, 1, buf);
    sp = (SUPER*)buf;

    // check EXT2 FS magic number;
    if (0xef53 == sp->s_magic)
    {
        printf("************  super block info:  ************\n");
        // print other fields of SUPER block
        InodesCount = sp->s_inodes_count;
        printf("inodes_count               %5d\n", InodesCount);
        BlocksCount = sp->s_blocks_count;
        printf("blocks_count               %5d\n", BlocksCount);
        printf("r_blocks_count             %5d\n", sp->s_r_blocks_count);
        printf("free_inodes_count          %5d\n", sp->s_free_inodes_count);
        printf("free_blocks_count          %5d\n", sp->s_free_blocks_count);
        printf("log_block_size             %5d\n", sp->s_log_block_size);
        printf("first_data_block           %5d\n", sp->s_first_data_block);
        printf("magic = %5x\n", sp->s_magic);
        printf("rev_level                  %5d\n", sp->s_rev_level);
        printf("inode_size                 %5d\n", sp->s_inode_size);
        printf("block_group_nr             %5d\n", sp->s_block_group_nr);
        printf("blksize                    %5d\n", EXT2_BLOCK_SIZE(sp));
        printf("inodes_per_group           %5d\n", sp->s_inodes_per_group);
        printf("---------------------------------------------\n");
        printf("desc_per_block             %5d\n", EXT2_DESC_PER_BLOCK(sp));
        printf("inodes_per_block           %5d\n", EXT2_INODES_PER_BLOCK(sp));
        //printf("inode_size_ratio           %5d\n");
        return fd;
    }

    return 0;
}

INODE* _iget(int fd, int ino)
{
    int blk, offset;
    char buf[BLKSIZE];

    blk = (ino - 1)/8 + InodesBeginBlock;
    offset = (ino - 1) % 8;
    //printf("ino %d  blk %d  offset %d\n", ino, blk, offset);

    get_block(fd, blk, buf);
    ip = (INODE*)buf + offset;
    myinode = *ip;

    return &myinode;
}

MINODE* iget (int fd, int ino)
{
    int i;
    char buf[BLKSIZE];

    // search minode[] for an entry with (dev,ino) AND refCount > 0;
    for (i = 0; i < MSIZE; i++)
    {
        if (minode[i].dev == fd && minode[i].ino == ino && minode[i].refCount > 0)
        {
            minode[i].refCount++;
            return &minode[i];
        }
    }

    ip = _iget(fd, ino);

    // not found, use a FREE minode[i] to load the INODE of (dev, ino)
    for (i = 0; i < MSIZE; i++)
    {
        // FREE minode
        if (minode[i].refCount == 0)
        {
            minode[i].inode = *ip; // load INODE of (dev,ino)
            minode[i].dev = fd;
            minode[i].ino = ino;
            minode[i].refCount = 1;
            minode[i].dirty = 0; // modified
            return &minode[i];
        }
    }

    return NULL; // minode is full!
}


void iput (MINODE* mip)
{
    int blk, offset;

    // Dispose of an minode pointed by mip:
    mip->refCount--;

    if (mip->refCount > 0) { return; }
    if (!mip->dirty) { return; }


    if (mip->refCount == 0 && mip->dirty)
    {
        // Write INODE back to the disk by its (dev, ino)
        blk = (mip->ino - 1)/8 + InodesBeginBlock;
        offset = (mip->ino - 1) % 8;
        lseek(mip->dev, (long)(blk*BLKSIZE), 0);
        write(mip->dev, mip->inode, BLKSIZE);
        mip->dirty = 0;
    }
}


// mount root file system, establish / and CWDs
void mount_root ()
{
    char buf[BLKSIZE];

    // open device for RW
    fd = open(device, O_RDWR);

    if (fd < 0)
    {
        printf("open %s failed\n", device);
        exit(1);
    }

    // read SUPER block to verify it's an EXT2 FS
    if (!(super()))
    {
        printf("error reading virtual disk\n");
        exit(1);
    }

    // get InodesBeginBlock
    get_block(fd, 2, buf);
    gp = (GD*)buf;
    InodesBeginBlock = gp->bg_inode_table;

    root = iget(fd, 2); /* get root inode */

    // Let cwd of both P0 and P1 point at the root minode (refCOunt = 3)
    proc[0].cwd = iget(fd, 2);
    proc[1].cwd = iget(fd, 2);
}

int search (INODE* ip, char* name)
{
    int i;
    char* cp;
    char buf[BLKSIZE];
    char temp[256];

    for (i = 0; i < 12; i++)
    {
        if (ip->i_block[i] == 0)
            break;

        get_block(fd, ip->i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;

        //printf("i=%d i_block[%d]=%d\n\n", i, i, ip->i_block[i]);
        //printf("   i_number rec_len name_len   name\n");

        while (cp < (buf + BLKSIZE))
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            //printf("   %5d    %4d    %4d       %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

            if (0 == strcmp(name, temp))
            {
                //printf("found %s : ino = %d\n", temp, dp->inode);
                return dp->inode;
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return BAD;
}


u32 getino (int* dev, char* pathname)
{
    MINODE* mip;
    int n, i, ino;
    char* name[12];
    char* token;
    token = strtok(pathname, "/");
    n = 0;
    while (token != 0)
    {
        name[n++] = token;
        token = strtok(0, pathname);
    }

    if (n == 0) return BAD;

    ino = 2;
    for (i = 0; i < n; i++)
    {
        mip = iget(fd, ino);
        //printf("=============================================\n");
        //printf("i=%d name[%d]=%s\n", i, i, name[i]);
        //printf("search for %s in %x\n", name[i], (u32)&(mip->inode));
        ino = search(&(mip->inode), name[i]);
        if ((ino == 0) || ((dp->file_type != EXT2_FT_DIR) && ((i + 1) < n)))
            return 0;
    }
    *dev = mip->dev;
    return dp->inode;
}

int change_dir (char* pathname)
{
    int ino, dev;
    MINODE* mip;

    if (pathname[0] == 0)
    {
        iput(running->cwd); // dispose of cwd
        running->cwd = root;
        root->refCount++;
        return GOOD;
    }

    ino = getino(&dev, pathname); // DOUBLE CHECK THIS CODE

    if (ino == 0)
    {
        printf("Directory not found\n");
        return BAD;
    }

    mip = iget(dev, ino);

    // Check it's a directory
    if ((mip->inode.i_mode & 0040000) != 0040000)
    {
        iput(mip);
        printf("Not a directory\n");
        return BAD;
    }

    // Dispose of original running->cwd
    running->cwd = mip;

    return GOOD;
}


int do_stat (char* pathname, struct stat* stPtr)
{
    int ino, dev;
    MINODE* mip;
    char dir_string[300];
    char buff[64];

    ino = getino(&dev, pathname);
    if (ino == BAD) return BAD;
    mip = iget(dev, ino);

    stPtr->st_dev = dev;
    stPtr->st_ino = ino;

    stPtr->st_mode = mip->inode.i_mode;
    stPtr->st_nlink = mip->inode.i_links_count;
    stPtr->st_uid = mip->inode.i_uid;
    stPtr->st_gid = mip->inode.i_gid;
    stPtr->st_rdev = 0;
    stPtr->st_size = mip->inode.i_size;
    stPtr->st_blksize = BLKSIZE;
    stPtr->st_blocks = mip->inode.i_blocks;
    stPtr->st_atime = mip->inode.i_atime;
    stPtr->st_mtime = mip->inode.i_mtime;
    stPtr->st_ctime = mip->inode.i_ctime;

    if ((stPtr->st_mode & 0100000) == 0100000) printf("-");
    if ((stPtr->st_mode & 0040000) == 0040000) printf("d");
    if ((stPtr->st_mode & 0120000) == 0120000) printf("l");

    if (stPtr->st_mode & (1 << 8)) printf("r"); else printf("-");
    if (stPtr->st_mode & (1 << 7)) printf("w"); else printf("-");
    if (stPtr->st_mode & (1 << 6)) printf("x"); else printf("-");
    if (stPtr->st_mode & (1 << 5)) printf("r"); else printf("-");
    if (stPtr->st_mode & (1 << 4)) printf("w"); else printf("-");
    if (stPtr->st_mode & (1 << 3)) printf("x"); else printf("-");
    if (stPtr->st_mode & (1 << 2)) printf("r"); else printf("-");
    if (stPtr->st_mode & (1 << 1)) printf("w"); else printf("-");
    if (stPtr->st_mode & (1 << 0)) printf("x"); else printf("-");

    strcpy(dir_string, ctime (&stPtr->st_mtime));
    dir_string[strlen(dir_string) - 1] = 0;
    printf(" %3d %4d %4d %6ld %s  %s\n", stPtr->st_nlink, stPtr->st_uid, stPtr->st_gid, stPtr->st_size, dir_string, pathname);
    iput(mip);

    return GOOD;
}


void _pwd(MINODE* wd)
{
    char buf[BLKSIZE];
    char tmp[BLKSIZE];
    char *cp = buf;
    int ino;
    __u32 iBlock;
    if (wd->ino == root->ino)
    {
        printf("/");
        return;
    }

    ip = &(wd->inode);
    iBlock = ip->i_block[0];

    get_block(fd, ip->i_block[0], buf);
    dp = (DIR*)buf;
    while(cp < buf + BLKSIZE)
    {
        strcpy(tmp, dp->name);
        tmp[dp->name_len] = 0;
        printf("%s\n", tmp);
        if (strcmp(tmp, "..") == 0)
        {
            printf("Found parent inode\n");
            ino = dp->inode;
            break;
        }
        cp += dp->rec_len;
        dp = (DIR*)cp;
    }

    wd = iget(wd->dev, ino);
    //exit(-1);
    _pwd(wd);

    ip = &(wd->inode);
    get_block(fd, ip->i_block[0], buf);
    cp = buf;
    dp = (DIR*)buf;
    while(cp < buf + BLKSIZE)
    {
        if (dp->inode == ino)
        {
            strcpy(tmp, dp->name);
            tmp[dp->name_len] = 0;
            printf("%s/", tmp);
            break;
        }
        cp += dp->rec_len;
        dp = (DIR*)cp;
    }
}

int do_pwd ()
{
    _pwd(running->cwd);
    printf("\n");
    return GOOD;
}

void list_dir (char* pathname)
{
    char buf[BLKSIZE];
    char tmp[BLKSIZE];
    char *cp = buf;
    struct stat stPtr;
    int ino, dev;
    MINODE* mip;
    DIR   *dp;

    // use cwd
    if (pathname[0] == 0)
    {
        ip = &(running->cwd->inode);
        get_block(fd, ip->i_block[0], buf);
        cp = buf;
        dp = (DIR*)buf;
        while (cp < buf + BLKSIZE)
        {
            strcpy(tmp, dp->name);
            tmp[dp->name_len] = 0;
            do_stat(tmp, &stPtr);
            //printf("   %5d    %4d    %4d       %s\n", dp->inode, dp->rec_len, dp->name_len, tmp);
            cp += dp->rec_len;
            dp = (DIR*)cp;
            //exit(-1);
        }
        return;
    }

    ino = getino(&dev, pathname);
    if (ino == BAD) return;
    mip = iget(fd, ino);
    ip = &(mip->inode);
    get_block(fd, ip->i_block[0], buf);
    cp = buf;
    dp = (DIR*)buf;
    while (cp < buf + BLKSIZE)
    {
        strcpy(tmp, dp->name);
        tmp[dp->name_len] = 0;
        if (strcmp(tmp, "lost+found") != 0) do_stat(tmp, &stPtr);
        //printf("   %5d    %4d    %4d       %s\n", dp->inode, dp->rec_len, dp->name_len, tmp);
        cp += dp->rec_len;
        dp = (DIR*)cp;
        //exit(-1);
    }
    return;
}

char pathName[256];
int main (int argc, char* argv[])
{
    struct stat mystat;

    bzero(pathName, 256);
    init();

    printf("Mounting...\n");
    mount_root();
    printf("hit a key to continue :"); getchar();

    printf("\nList Directory\n");
    strcpy(pathName, "");
    list_dir(pathName);

    printf("\nChange Directory /cs260\n");
    strcpy(pathName, "/cs260");
    change_dir(pathName);
    printf("hit a key to continue :"); getchar();

    printf("\nStat /cs560\n");
    strcpy(pathName, "/cs560");
    do_stat(pathName, &mystat);

    //do_pwd();

    return 0;
} /* end main */
