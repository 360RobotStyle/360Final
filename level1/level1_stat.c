#include "../filesystem.h"
#include "../util.h"
#include <time.h>

void
stat_file()
{
    MINODE* mip;
    char temp[64];
    u32 ino;
    int dev;
    time_t t;

    if
    (    0 == strlen(    pathName    )    )
    {
        ino = running->cwd->ino;
        dev = running->cwd->dev;
    }
    else if (0 == strcmp(pathName, "/"))
    {
        ino = root->ino;
        dev = root->dev;
    }
    else
    {
        ino = getino(&dev, pathName);
    }

    ino = getino(&dev, pathName);
    if ((u32)-1 == ino)
    {
        ino = getino(&dev, dir_name(pathName));
        mip = iget(dev, ino);
        ino = getfileino(mip, base_name(pathName));
        iput(mip);
    }
    mip = iget(dev, ino);

    t = mip->INODE.i_mtime;
    strcpy(temp, ctime(&t));
    temp[strlen(temp) - 9] = 0;

    printf("*********  stat **********\n");
    printf("dev=%d   ino=%ld   mod=%x\n", dev, ino, mip->INODE.i_mode);
    printf("uid=%d   gid=%d   nlink=%d\n", mip->INODE.i_uid, mip->INODE.i_gid, mip->INODE.i_links_count);
    printf("size=%d time=%s\n", mip->INODE.i_size, temp);
    printf("**************************\n");

    iput(mip);
}

