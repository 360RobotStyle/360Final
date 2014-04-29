#include "../filesystem.h"
#include "../util.h"

void
do_cd()
{
    MINODE *mip;
    u32 targetino;
    int dev;
    u32 myino, parentino;
    strncpy(pathNameTokenized, pathName, strlen(pathName) + 1);
    findino(running->cwd, &myino, &parentino);
    targetino = getino(&dev, pathName);
    if (-1 != targetino)
    {
        mip = iget(dev, targetino);
        iput(running->cwd);
        running->cwd = mip;
    }
    else
    {
        printf("Couldn't get the inode for '%s'\n", pathName);
    }
}
