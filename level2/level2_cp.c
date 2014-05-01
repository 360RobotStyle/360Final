#include "../filesystem.h"
#include "../util.h"

static void
mywrite2(OFT* oftp, char* buf, int nbytes)
{
    MINODE* mip;
    int lbk, startByte, remain, blk;
    char wbuf[BLOCK_SIZE];
    char* cq;
    int i, j;
    int tbytes = 0;
    u32 i_buf[BLOCK_SIZE / 4]; // indirect buf
    u32 di_buf[BLOCK_SIZE / 4]; // double indirect buf

    cq = buf;
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
    printf("wrote %d char into file\n", tbytes);
}

static int
my_creat2(MINODE* pip, char* name)
{
    int i;
    MINODE* mip;
    u32 inumber, bnumber;
    char buf[BLOCK_SIZE];
    char* cp;
    int need_length, ideal_length, rec_len;

    // allocate an inode and disk block for the new dir
    inumber = ialloc(pip->dev);

    // to load INODE into a minode[],
    mip = iget(pip->dev, inumber);
    //printf("Inumber %d Bnumber %d\n", inumber, bnumber);
    // write contents into the intended INODE in memory
    mip->INODE.i_mode = D_FILE_MODE;
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->INODE.i_size = 0;

    mip->INODE.i_links_count = 1;
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);

    mip->INODE.i_blocks = 0;
    for (i = 0; i < 15; i++)
        mip->INODE.i_block[i] = 0;
    mip->dirty = 1;

    iput(mip);

    // enter name into parent's directory
    need_length = 4 * ((8 + strlen(name) + 3) / 4);

    //put_rec(pip, name, inumber);
    for (i = 0; i < 12 && (pip->INODE).i_block[i]; i++)
    {
        // read parent's data block into buf[]
        get_block(pip->dev, (pip->INODE).i_block[i], buf);
        cp = buf;
        dp = (DIR*)buf;
        while (cp < (buf + BLOCK_SIZE))
        {
            dp = (DIR*)cp;
            cp += dp->rec_len;
        }
        cp -= dp->rec_len;

        // dp now points to the LAST entry
        ideal_length = 4 * ((8 + dp->name_len + 3) / 4);
        rec_len = dp->rec_len;
        if (rec_len - ideal_length >= need_length)
        {
            // enter the new entry as the LAST entry and trim the previous
            // entry to its ideal length
            dp->rec_len = ideal_length;
            cp += dp->rec_len;
            dp = (DIR*)cp;
            dp->inode = inumber;
            dp->name_len = strlen(name);
            dp->file_type = EXT2_FT_REG_FILE;
            strncpy(dp->name, name, dp->name_len);
            dp->rec_len = rec_len - ideal_length;
            put_block(pip->dev, (pip->INODE).i_block[i], buf);
            break;
        }
        else
        {
            if (0 == (pip->INODE).i_block[i + 1])
            {
                // allocate a new data block
                // enter the new entry as the first entry in the new data block
                get_block(pip->dev, (pip->INODE).i_block[i + 1], buf);
                dp = (DIR*)buf;
                dp->inode = inumber;
                dp->name_len = strlen(name);
                dp->file_type = EXT2_FT_REG_FILE;
                strncpy(dp->name, name, dp->name_len);
                dp->rec_len = BLOCK_SIZE;
                put_block(pip->dev, (pip->INODE).i_block[i + 1], buf);
                break;
            }
        }
    }

    pip->INODE.i_atime = time(0L);
    pip->dirty = 1;

    iput(pip);
    return 0;
}

int
creat_file2()
{
    int dev;
    u32 ino;
    char* parent;
    char* child;
    MINODE* pip;

    if ('/' == parameter[0])
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    parent = dir_name(parameter);
    child = base_name(parameter);

    if (0 == strcmp(child, "."))
    {
        return -1;
    }
    ino = getino(&dev, parent);
    pip = iget(dev, ino);

    // verify parent INODE is a DIR and child does not exist in the parent dir
    if ((0040000 == (pip->INODE.i_mode & 0040000)) && (0 == is_exist(pip, child)))
    {
        return my_creat2(pip, child);
    }

    return -1;
}

void
cp_file()
{
    char rbuf[BLOCK_SIZE], dummy = 0;
    char wbuf[BLOCK_SIZE];
    OFT *oftp1, *oftp2;
    MINODE *mip1, *mip2;
    u32 ino;
    int dev;
    int mode;
    int i, n, fd;


    // 1)
    if (0 == strcmp(pathName, ""))
    {
        printf("cp : missing source file name\n");
        return;
    }

    ino = getino(&dev, dir_name(pathName));
    if (-1 == ino)
    {
        printf("cp : couldn't find source path '%s'\n", dir_name(pathName));
        return;
    }
    mip1 = iget(dev, ino);
    ino = getfileino(mip1, base_name(pathName));
    iput(mip1);
    if (-1 == ino)
    {
        printf("cp : couldn't find source filename '%s'\n", base_name(pathName));
        return;
    }

    mip1 = iget(dev, ino);

    if ((mip1->INODE.i_mode & 0100000) != 0100000)
    {
        printf("cp : invalid source file type\n");
        return;
    }

    if (-1 == creat_file2())
    {
        printf("cp : problem with destination file\n");
        return;
    }

    ino = getino(&dev, dir_name(parameter));
    mip2 = iget(dev, ino);
    ino = getfileino(mip2, base_name(parameter));
    iput(mip2);
    mip2 = iget(dev, ino);

    // Source
    oftp1 = falloc();       // get a FREE OFT
    if (0 == oftp1) return; // oft FULL
    oftp1->mode = 0;        // READ mode
    oftp1->refCount = 1;
    oftp1->inodeptr = mip1;  // point at the file's minode[]
    oftp1->offset = 0;

    // Destination
    oftp2 = falloc();       // get a FREE OFT
    if (0 == oftp2) return; // oft FULL
    oftp2->mode = 1;        // READ mode
    oftp2->refCount = 1;
    oftp2->inodeptr = mip2;  // point at the file's minode[]
    oftp2->offset = 0;

    while (n = myread2(oftp1, rbuf, BLOCK_SIZE))
    {
        strncpy(wbuf, rbuf, n);
        mywrite2(oftp2, wbuf, n);
    }

    mip2->INODE.i_mtime = time(0L);
    mip2->dirty = 1;
    iput(mip2);
    iput(mip1);
}
