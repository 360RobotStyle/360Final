#include "../filesystem.h"
#include "../util.h"

void
do_close()
{
    MINODE *pip;
    MINODE *mip;
    OFT *oftp;
    int dev;
    int i;

    igetparentandfile(&dev, &pip, &mip, pathName);

    if (!pip)
    {
        printf("Couldn't find file '%s'\n", pathName);
        return;
    }
    else if (!mip)
    {
        printf("Couldn't find file '%s'\n", pathName);
        iput(pip);
        return;
    }
    iput(pip);

    // Check all fd entries.
    for (i = 0; i < NFD; i++)
    {
        if (running->fd[i] && running->fd[i]->refCount && (running->fd[i]->inodeptr == mip))
        {
            (running->fd[i])->refCount--;
            iput(mip); // mip has a matching ref count.
            printf("Closed '%s'. File has %i open file descriptors.\n", pathName,
                    (running->fd[i])->refCount);
            return;
        }
    }
    printf("File '%s' is not open.\n", pathName);
    iput(mip);
}
