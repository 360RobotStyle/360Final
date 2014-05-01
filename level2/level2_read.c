#include "../filesystem.h"
#include "../util.h"

static void
myread(int fd, char* buf, int nbytes)
{
    OFT* oftp;
    int count, avil, blk;
    int lbk, startByte, remain;
    char* cq;
    char* cp;
    char readbuf[BLOCK_SIZE];
    MINODE* mip;
    u32 i_buf[BLOCK_SIZE / 4]; // indirect buf
    u32 di_buf[BLOCK_SIZE / 4]; // double indirect buf

    oftp = running->fd[fd];
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

    printf("\nread : read %d char from file %d\n", count, fd);
}

void
read_file()
{
    int fd;
    int i;
    int nbytes;
    char* buf;

    if (1 == strlen(pathName) && pathName[0] >= '0' && pathName[0] <= '9')
    {
        if (0 == strlen(parameter))
        {
            printf("read : bad nbytes\n");
            return;
        }

        for (i = 0; i < strlen(parameter); i++)
        {
            if (!isdigit(parameter[i]))
            {
                printf("read : invalid bytes\n");
                return;
            }
        }

        // 1) get fd & nbytes
        fd = atoi(pathName);
        nbytes = atoi(parameter);

        // 2) verify fd is indeed opened for R or RW mode
        if ((0 != running->fd[fd]) && (0 != running->fd[fd]->refCount) &&
            (0 == running->fd[fd]->mode || 1 == running->fd[fd]->mode))
        {
            buf = (char*)malloc(sizeof(char) * nbytes + 1);
            bzero(buf, nbytes + 1);
            myread(fd, buf, nbytes);
            printf("\033[32m%s\033[0m\n", buf);
            free(buf);
        }
        else
        {
            printf("read : fd = %d is NOT FOR READ\n", fd);
        }
    }
    else
    {
        printf("read : ensure correct fd is provided\n");
    }
}
