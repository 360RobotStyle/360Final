#include "../filesystem.h"
#include "../util.h"

static void
mywrite(int fd, char* buf, int nbytes)
{
    OFT* oftp;
    MINODE* mip;
    int lbk, startByte, remain, blk;
    char wbuf[BLOCK_SIZE];
    char* cq;

    cq = buf;
    oftp = running->fd[fd];
    mip = oftp->inodeptr;

    while (nbytes > 0)
    {
        // compute LOGICAL BLOCK (lblk) and the startByte in the lbk
        lbk = oftp->offset / BLOCK_SIZE;
        startByte = oftp->offset % BLOCK_SIZE;

        // Only works for DIRECT data blocks
        // FIXME figure out how to write indirect and double-indirect blocks

        // direct block
        if (lbk < 12)
        {
            if (mip->INODE.i_block[lbk] == 0)
            {
                mip->INODE.i_block[lbk] = balloc(mip->dev);
                // OPTIONAL: write a block of 0's to blk on disk
            }
            blk = mip ->INODE.i_block[lbk];
        }
        // indirect block
        else if (lbk >= 12 && lbk < 256 + 12)
        {
            // FIXME

        }
        // double indirect block
        else
        {
            // FIXME
        }



        /* all cases come to here : write to the data block */
        get_block(mip->dev, blk, wbuf);    // read disk block into wbuf[]
        char* cp = wbuf + startByte;       // cp points at startByte in wbuf[]
        remain = BLOCK_SIZE - startByte;   // # of bytes remain in this block

        while (remain > 0)                 // write as much as remain allows
        {
            *cp++ = *cq++;
            nbytes--;
            remain--;
            oftp->offset++;
            if (oftp->offset > mip->INODE.i_size) // especially for RW|APPEND mode
                mip->INODE.i_size++;
            if (nbytes <= 0) break;
        }
        put_block(mip->dev, blk, wbuf);     // write wbuf[] to disk

        // loop back to while to write more ... until nbytes are written
    }

    mip->dirty = 1;
    printf("wrote %d char into file fd=%d\n", nbytes, fd);
}

void
write_file()
{
    int fd;
    int nbytes;
    char buf[BLOCK_SIZE];

    if (1 == strlen(pathName) && pathName[0] >= '0' && pathName[0] <= '9')
    {
        // 1) get fd
        fd = atoi(pathName);

        // 2) verify fd is indeed opened for W or RW or APPEND mode
        if ((0 != running->fd[fd]) && (0 != running->fd[fd]->refCount) &&
            (1 == running->fd[fd]->mode ||
             2 == running->fd[fd]->mode ||
             3 == running->fd[fd]->mode))
        {
            // 3) copy text string into a buf[] and get it's length as nbytes
            nbytes = strlen(parameter);
            strncpy(buf, parameter, nbytes + 1);
            mywrite(fd, buf, nbytes);
        }
        else
        {
            printf("write : fd = %d is NOT FOR WRITE\n", fd);
        }
    }
    else
    {
        printf("write : ensure correct fd is provided\n");
    }
}
