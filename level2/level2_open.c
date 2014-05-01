#include "../filesystem.h"
#include "../util.h"

void
truncate(MINODE* mip)
{
    int i;

    // 1) FIXME release mip->INODE's data blocks
    // a file may have 12 direct blocks, 256 indirect blocks and 256* 256
    // double indirect data blocks. release them all

    // Direct block
    for (i = 0; i < 12 && mip->INODE.i_block[i]; i++)
        mip->INODE.i_block[i] = 0;
    // Indirect block
    if (mip->INODE.i_block[12])
        mip->INODE.i_block[12] = 0;
    // Double indirect block
    if (mip->INODE.i_block[13])
        mip->INODE.i_block[13] = 0;

    // 2) update INODE's time field
    mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);

    // 3) set INODE's size to 0 and mark Minode[] dirty
    mip->INODE.i_size = 0;
    mip->dirty = 1;
}

void
do_open()
{

    OFT* oftp;
    MINODE *mip;
    u32 ino;
    int dev;
    int mode;
    int i;


    // ask for a pathname and mode to open:
    // You may use mode = 0|1|2|3 for R|W|RW|APPEND
    if (0 == strcmp(pathName, ""))
    {
        printf("open : missing filename\n");
        return;
    }

    if (1 == strlen(parameter) && parameter[0] >= '0' && parameter[0] <= '3')
    {
        mode = atoi(parameter);
    }
    else
    {
        printf("open: invalid mode\n");
        return;
    }

    // 2. get pathname's inumber:
    ino = getino(&dev, dir_name(pathName));
    if (-1 == ino)
    {
        printf("open : couldn't find path '%s'\n", dir_name(pathName));
        return;
    }
    mip = iget(dev, ino);
    ino = getfileino(mip, base_name(pathName));
    iput(mip);
    if (-1 == ino)
    {
        printf("open : couldn't find filename '%s'\n", base_name(pathName));
        return;
    }

    // 3. get its Minode pointer
    mip = iget(dev, ino);

    // 4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
    // (Optional : do NOT check FILE type so that we can open DIRs for RW)
    if ((mip->INODE.i_mode & 0100000) != 0100000)
    {
        printf("open : invalid file type\n");
        return;
    }

    // FIXME Need to check for permissions

    // FIXME Check whether the file is ALREADY opened with INCOMPATIBLE type:
    // If it's already opened for W, RW, APPEND : reject.
    // (that is, only multiple R are OK)

    // 5. allocate an OpenFileTable (OFT) entry and fill in values:
    oftp = falloc();       // get a FREE OFT
    if (0 == oftp) return; // oft FULL
    oftp->mode = mode;     // open mode
    oftp->refCount = 1;
    oftp->inodeptr = mip;  // point at the file's minode[]

    // 6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:
    switch (mode)
    {
        case 0 : oftp->offset = 0; break;
        case 1 : truncate(mip);             // W truncate file to 0 size
                 oftp->offset = 0; break;
        case 2 : oftp->offset = 0; break;   // RW does NOT truncate file
        case 3 : oftp->offset =  mip->INODE.i_size; break;  // APPEND mode
        default: printf("open : invalid mode\n");
                 return;
    }

    // 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
    // Let running->fd[i] point at the OFT entry
    for (i = 0; i < NFD; i++)
    {
        if (0 == running->fd[i])
        {
            running->fd[i] = oftp;
            break;
        }
    }

    // 8. update INODE's time field.
    // for W|RW|APPEND mode : mark Minode[] dirty
    if (1 == mode || 2 == mode || 3 == mode)
    {
        mip->INODE.i_atime = mip->INODE.i_mtime = time(0L); // just access time???
        mip->dirty = 1;
    }

    // 9. return i as the file descriptor
}
