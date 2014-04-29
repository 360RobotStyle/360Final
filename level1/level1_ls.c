#include "../filesystem.h"
#include "../util.h"
#include <time.h>

static void
file_info (int dev, u32 ino)
{
    MINODE* mip;
    char temp[300];
    char buff[64];
    time_t t;

    mip = iget(dev, ino);

    if ((mip->INODE.i_mode & 0100000) == 0100000) printf("-");
    if ((mip->INODE.i_mode & 0040000) == 0040000) printf("d");
    if ((mip->INODE.i_mode & 0120000) == 0120000) printf("l");

    if (mip->INODE.i_mode & (1 << 8)) printf("r"); else printf("-");
    if (mip->INODE.i_mode & (1 << 7)) printf("w"); else printf("-");
    if (mip->INODE.i_mode & (1 << 6)) printf("x"); else printf("-");
    if (mip->INODE.i_mode & (1 << 5)) printf("r"); else printf("-");
    if (mip->INODE.i_mode & (1 << 4)) printf("w"); else printf("-");
    if (mip->INODE.i_mode & (1 << 3)) printf("x"); else printf("-");
    if (mip->INODE.i_mode & (1 << 2)) printf("r"); else printf("-");
    if (mip->INODE.i_mode & (1 << 1)) printf("w"); else printf("-");
    if (mip->INODE.i_mode & (1 << 0)) printf("x"); else printf("-");

    t = mip->INODE.i_mtime;
    strcpy(temp, ctime (&t));
    temp[strlen(temp) - 1] = 0;
    printf(" %3d %4d %4d %6d %s  ", mip->INODE.i_links_count,
                                     mip->INODE.i_uid,
                                     mip->INODE.i_gid,
                                     mip->INODE.i_size, temp);
    iput(mip);
}

void
do_ls()
{
    int dev;
    u32 ino;
    MINODE* mip;
    char buf[BLOCK_SIZE];
    char temp[BLOCK_SIZE];
    char* cp;

    if (0 == pathName[0])
    {
        printf("ls : current working directory");
        ino = running->cwd->ino;
        dev = running->cwd->dev;
    }
    else if (0 == strcmp(pathName, "/"))
    {
        printf("ls : root directory");
        ino = root->ino;
        dev = root->dev;
    }
    else
    {
        printf("ls : ");
        ino = getino(&dev, pathName);
    }

    if (-1 == ino)
    {
        printf("it is not a directory\n");
        return;
    }
    printf("\n");

    mip = iget(dev, ino);
    ip = &(mip->INODE);
    get_block(mip->dev, ip->i_block[0], buf);
    cp = buf;
    dp = (DIR*)buf;
    while (cp < buf + BLOCK_SIZE)
    {
        strcpy(temp, dp->name);
        temp[dp->name_len] = 0;
        file_info(dev, dp->inode);
        printf("%s\n", temp);
        cp += dp->rec_len;
        dp = (DIR*)cp;
    }
    iput(mip);
}

//void list_dir (char* pathname)
//{
    //char buf[BLOCK_SIZE];
    //char tmp[BLOCK_SIZE];
    //char *cp = buf;
    //struct stat stPtr;
    //int ino, dev;
    //MINODE* mip;
    //DIR   *dp;

    //// use cwd
    //if (pathname[0] == 0)
    //{
        //ip = &(running->cwd->inode);
        //get_block(fd, ip->i_block[0], buf);
        //cp = buf;
        //dp = (DIR*)buf;
        //while (cp < buf + BLOCK_SIZE)
        //{
            //strcpy(tmp, dp->name);
            //tmp[dp->name_len] = 0;
            //do_stat(tmp, &stPtr);
            ////printf("   %5d    %4d    %4d       %s\n", dp->inode, dp->rec_len, dp->name_len, tmp);
            //cp += dp->rec_len;
            //dp = (DIR*)cp;
            ////exit(-1);
        //}
        //return;
    //}

    //ino = getino(&dev, pathname);
    //if (ino == BAD) return;
    //mip = iget(fd, ino);
    //ip = &(mip->inode);
    //get_block(fd, ip->i_block[0], buf);
    //cp = buf;
    //dp = (DIR*)buf;
    //while (cp < buf + BLOCK_SIZE)
    //{
        //strcpy(tmp, dp->name);
        //tmp[dp->name_len] = 0;
        //if (strcmp(tmp, "lost+found") != 0) do_stat(tmp, &stPtr);
        ////printf("   %5d    %4d    %4d       %s\n", dp->inode, dp->rec_len, dp->name_len, tmp);
        //cp += dp->rec_len;
        //dp = (DIR*)cp;
        ////exit(-1);
    //}
    //return;
//}


//int do_stat (char* pathname, struct stat* stPtr)
//{
    //int ino, dev;
    //MINODE* mip;
    //char dir_string[300];
    //char buff[64];

    //ino = getino(&dev, pathname);
    //if (ino == BAD) return BAD;
    //mip = iget(dev, ino);

    //stPtr->st_dev = dev;
    //stPtr->st_ino = ino;

    //stPtr->st_mode = mip->inode.i_mode;
    //stPtr->st_nlink = mip->inode.i_links_count;
    //stPtr->st_uid = mip->inode.i_uid;
    //stPtr->st_gid = mip->inode.i_gid;
    //stPtr->st_rdev = 0;
    //stPtr->st_size = mip->inode.i_size;
    //stPtr->st_BLOCK_SIZE = BLOCK_SIZE;
    //stPtr->st_blocks = mip->inode.i_blocks;
    //stPtr->st_atime = mip->inode.i_atime;
    //stPtr->st_mtime = mip->inode.i_mtime;
    //stPtr->st_ctime = mip->inode.i_ctime;

    //if ((stPtr->st_mode & 0100000) == 0100000) printf("-");
    //if ((stPtr->st_mode & 0040000) == 0040000) printf("d");
    //if ((stPtr->st_mode & 0120000) == 0120000) printf("l");

    //if (stPtr->st_mode & (1 << 8)) printf("r"); else printf("-");
    //if (stPtr->st_mode & (1 << 7)) printf("w"); else printf("-");
    //if (stPtr->st_mode & (1 << 6)) printf("x"); else printf("-");
    //if (stPtr->st_mode & (1 << 5)) printf("r"); else printf("-");
    //if (stPtr->st_mode & (1 << 4)) printf("w"); else printf("-");
    //if (stPtr->st_mode & (1 << 3)) printf("x"); else printf("-");
    //if (stPtr->st_mode & (1 << 2)) printf("r"); else printf("-");
    //if (stPtr->st_mode & (1 << 1)) printf("w"); else printf("-");
    //if (stPtr->st_mode & (1 << 0)) printf("x"); else printf("-");

    //strcpy(dir_string, ctime (&stPtr->st_mtime));
    //dir_string[strlen(dir_string) - 1] = 0;
    //printf(" %3d %4d %4d %6ld %s  %s\n", stPtr->st_nlink, stPtr->st_uid, stPtr->st_gid, stPtr->st_size, dir_string, pathname);
    //iput(mip);

    //return GOOD;
//}

