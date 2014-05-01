#include "../filesystem.h"
#include "../util.h"

static int
myread2(OFT* oftp, char* buf, int nbytes)
{
    int count, avil, blk;
    int lbk, startByte, remain;
    char* cq;
    char* cp;
    char readbuf[BLOCK_SIZE];
    MINODE* mip;
    u32 i_buf[BLOCK_SIZE / 4]; // indirect buf
    u32 di_buf[BLOCK_SIZE / 4]; // double indirect buf

    mip = oftp->inodeptr;

    // 1) number of bytes still available in file
    avil = mip->INODE.i_size - oftp->offset;
    count = 0;
    cq = buf;

    // 2)
    while (nbytes && avil)
    {
        // compute LOGICAL BLOCK number lbk and startByte in that block from offset
        lbk = oftp->offset / BLOCK_SIZE;
        startByte = oftp->offset % BLOCK_SIZE;

        // Direct block
        if (lbk < 12)
        {
            blk = mip->INODE.i_block[lbk];
            //printf("direct %d\n", blk);
        }
        // indirect block
        else if (lbk >= 12 && lbk < 256 + 12)
        {
            get_block(mip->dev, mip->INODE.i_block[12], (char *) i_buf);
            //printf("indirect %d\n", i_buf[lbk - 12]);
            blk = i_buf[lbk - 12];
        }
        // double indirect block
        else
        {
            get_block(mip->dev, mip->INODE.i_block[13], (char *) i_buf);
            get_block(mip->dev, i_buf[((lbk - 12) / 256) - 1], (char *) di_buf);
            //printf ("double indirect %d\n", di_buf[(lbk - 12) - (256 * ((lbk - 12) / 256))]);
            blk = di_buf[(lbk - 12) - (256 * ((lbk - 12) / 256))];
        }

        // get the data block into readbuf[BLOCK_SIZE]
        get_block(mip->dev, blk, readbuf);

        // copy from startByte to buf[], at most remain bytes in this block
        cp = readbuf + startByte;
        remain = BLOCK_SIZE - startByte; // number of bytes remain in readbuf[]

        // FIXME Optimize this code for EXTRA CREDIT
        while (remain > 0)
        {
            *cq++ = *cp++;           // copy byte into buf[]
            oftp->offset++;
            count++;
            avil--;
            nbytes--;
            remain--;
            if (nbytes <= 0 || avil <= 0) break;
        }

        // if one data block is not enough, loop back to OUTER while for more
    }

    return count;
}

void
cat_file()
{
    char buf[BLOCK_SIZE], dummy = 0;
    OFT* oftp;
    MINODE *mip;
    u32 ino;
    int dev;
    int mode;
    int i, n, fd;


    // 1)
    if (0 == strcmp(pathName, ""))
    {
        printf("open : missing filename\n");
        return;
    }

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

    mip = iget(dev, ino);

    if ((mip->INODE.i_mode & 0100000) != 0100000)
    {
        printf("open : invalid file type\n");
        return;
    }

    oftp = falloc();       // get a FREE OFT
    if (0 == oftp) return; // oft FULL
    oftp->mode = 0;        // READ mode
    oftp->refCount = 1;
    oftp->inodeptr = mip;  // point at the file's minode[]
    oftp->offset = 0;

    printf("\033[32m");
    while (n = myread2(oftp, buf, BLOCK_SIZE))
    {

        for (i = 0; i < n; i++)
            putchar(buf[i]);
    }
    printf("\033[0m");

    oftp->refCount = 0;
    oftp->inodeptr = 0;
    iput(mip);
}
