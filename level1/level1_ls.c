#include "../filesystem.h"
#include "../util.h"

void
do_ls()
{
    //if (0 == pathName[0])
    //{
        //ip = &(running->cwd->INODE);
    //}
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

