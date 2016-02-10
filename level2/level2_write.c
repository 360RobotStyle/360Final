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
    int i, j;
    int tbytes = 0;
    u32 i_buf[BLOCK_SIZE / 4]; // indirect buf
    u32 di_buf[BLOCK_SIZE / 4]; // double indirect buf

    cq = buf;
    oftp = running->fd[fd];
    mip = oftp->inodeptr;

    while (nbytes > 0)
    {
        // compute LOGICAL BLOCK (lblk) and the startByte in the lbk
        lbk = oftp->offset / BLOCK_SIZE;
        startByte = oftp->offset % BLOCK_SIZE;

        // Only works for DIRECT data blocks

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
            get_block(mip->dev, mip->INODE.i_block[12], (char *) i_buf);
            for (i = 0; i < 256; i++)
            {
                if (0 == i_buf[i])
                {
                    i_buf[i] = balloc(mip->dev);
                    blk = i_buf[i];
                    put_block(mip->dev, mip->INODE.i_block[12], (char *) i_buf);
                    break;
                }
            }
        }
        // double indirect block
        else
        {
            blk = -1;
            get_block(mip->dev, mip->INODE.i_block[13], (char *) i_buf);
            for (i = 0; i < 256; i++)
            {
                if (i_buf[i])
                {
                    get_block(mip->dev, i_buf[i], (char *) di_buf);
                    for (j = 0; j < 256; j++)
                    {
                        if (0 == di_buf[j])
                        {
                            di_buf[j] = balloc(mip->dev);
                            blk = di_buf[j];
                            put_block(mip->dev, i_buf[i], (char *) di_buf);
                            break;
                        }
                    }
                    if (blk != -1) break;
                }
            }
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
            tbytes++;
            oftp->offset++;
            if (oftp->offset > mip->INODE.i_size) // especially for RW|APPEND mode
                mip->INODE.i_size++;
            if (nbytes <= 0) break;
        }
        put_block(mip->dev, blk, wbuf);     // write wbuf[] to disk

        // loop back to while to write more ... until nbytes are written
    }

    mip->dirty = 1;
    printf("wrote %d char into file fd=%d\n", tbytes, fd);
}

void
write_file()
{
    int fd;
    int nbytes;
    char buf[BLOCK_SIZE];

    bzero(buf, BLOCK_SIZE);
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
            printf("%s\n", buf);
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
