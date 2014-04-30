#include "../filesystem.h"
#include "../util.h"

void
do_open()
{
/*
    MINODE *pip;
    MINODE *mip;
    u32 pino;
    u32 mino;
    int dev;


    // ask for a pathname and mode to open:
    // You may use mode = 0|1|2|3 for R|W|RW|APPEND

    // 2. get pathname's inumber:
    pino = getino(&dev, dir_name(pathName));
    if (-1 == pino)
    {
        printf("Couldn't fine path '%s'\n", dir_name(pathName));
        return;
    }
    pip = iget(dev, pino);

    // 3. get its Minode pointer

    igetparentandfile(&dev, &mip, &pip, pathName);

    // 4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
    // (Optional : do NOT check FILE type so that we can open DIRs for RW)

    // Check whether the file is ALREADY opened with INCOMPATIBLE type:
    // If it's already opened for W, RW, APPEND : reject.
    // (that is, only multiple R are OK)

    // 5. allocate an OpenFileTable (OFT) entry and fill in values:
    oftp = falloc();       // get a FREE OFT
    oftp->mode = mode;     // open mode 
    oftp->refCount = 1;
    oftp->inodeptr = mip;  // point at the file's minode[]

    // 6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

    switch (mode)
    {
        case 0 : oftp->offset = 0; break;
        case 1 : truncate(mip);        // W : truncate file to 0 size
                 oftp->offset = 0; break;
        case 2 : oftp->offset = 0; break;   // RW does NOT truncate file
        case 3 : oftp->offset =  mip->INODE.i_size; break;  // APPEND mode
        default: printf("invalid mode\n");
                 return(-1);
    }

    // 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
    // Let running->fd[i] point at the OFT entry

    // 8. update INODE's time field. 
    // for W|RW|APPEND mode : mark Minode[] dirty

    // 9. return i as the file descriptor
    // */
}
