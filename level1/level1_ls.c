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
    int i;

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
    printf("INO: %ld\n", ino);
    if (1 == ino)
    {
        printf("it is not a directory\n");
        return;
    }
    printf("\n");

    mip = iget(dev, ino);
    ip = &(mip->INODE);
    i = 0;

    for (i = 0; i < 12 &&  (mip->INODE).i_block[i]; i++)
    {
        get_block(mip->dev, (mip->INODE).i_block[i], buf);
        cp = buf;
        dp = (DIR*)buf;
        printf("i_block is %i\n", (int) (mip->INODE).i_block[i]);
        while (cp < buf + BLOCK_SIZE && dp->rec_len)
        {
            if (0 == dp->rec_len) break;
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            file_info(dev, dp->inode);
            printf("%s %u\n", temp, dp->inode);
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }

    iput(mip);
}

