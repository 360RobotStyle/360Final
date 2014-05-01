#include "../filesystem.h"
#include "../util.h"

void
pfd()
{
    int i;
    OFT* oftp;
    int isOpenFile = 0;
    char filename[128] = "";

    for (i = 0; i < NFD; i++)
    {
        if (0 != running->fd[i])
            isOpenFile = 1;
    }

    if (isOpenFile)
    {
        printf("==================   pid = %d   ===================\n", running->pid);
        printf("fd   mode   count  offset  [dev,ino]    filename\n"
               "--   ----   -----  ------  ---------   ------------\n");
        for (i = 0; i < NFD; i++)
        {
            if (0 != running->fd[i])
            {
                oftp = running->fd[i];
                printf("\033[33m%2d\033[0m   ", i);
                switch (oftp->mode)
                {
                    case 0: printf("READ "); break;
                    case 1: printf("WRIT "); break;
                    case 2: printf("RDWR "); break;
                    case 3: printf("APND "); break;
                }
                // FIXME print filename
                printf("   %03d    %04d    [%2d, %2d]    %s\n", oftp->refCount,
                                                                oftp->offset,
                                                                oftp->inodeptr->dev,
                                                                oftp->inodeptr->ino, filename);
            }
        }
        printf("==================================================\n");
    }
    else
    {
        printf("pfd : no opened files\n");
    }
}
