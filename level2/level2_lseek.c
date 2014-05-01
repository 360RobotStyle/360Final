#include "../filesystem.h"
#include "../util.h"

void
do_lseek()
{
    MINODE *pip;
    MINODE *mip;
    OFT *oftp;
    int dev;
    int i;
    u32 offset;

    if (0 == strcmp(pathName, ""))
    {
        printf("lseek : missing filename\n");
        return;
    }

    if (0 == strcmp(parameter, ""))
    {
        printf("lseek : missing offset\n");
        return;
    }
    offset = (u32) atoi(parameter);

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
            if (offset > (mip->INODE).i_size)
            {
                printf("Offset exceeds file boundary\n");
            }
            else
            {
                printf("Moving offset from %i to %i\n", (int) (running->fd[i])->offset, (int) offset);
                (running->fd[i])->offset = offset;
            }
            iput(mip);

            return;
        }
    }
    printf("File '%s' is not open.\n", pathName);
    iput(mip);
}
