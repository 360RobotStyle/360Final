#include "../filesystem.h"
#include "../util.h"

void
do_cd()
{
    MINODE *mip;
    u32 targetino;
    int dev;
    strncpy(pathNameTokenized, pathName, strlen(pathName) + 1);
    targetino = getino(&dev, pathName);
    if (-1 != targetino)
    {
        mip = iget(dev, targetino);
        iput(running->cwd);
        if (mip->mounted)
        {
            running->cwd = mip->mountptr->mounted_inode;
            iput(mip);
            running->cwd->refCount++;
        }
        else
        {
            running->cwd = mip;
        }
    }
    else
    {
        printf("Couldn't get the inode for '%s'\n", pathName);
    }
}
