#include "util.h"

// util.h
//extern INODE myinode;
extern PROC proc[NPROC];
extern PROC* running;
extern PROC* readyQueue;
extern MINODE minode[NMINODES];
extern MINODE* root;
extern OFT oft[NOFT];
extern MOUNT mount[NMOUNT];

extern char line[256];
extern char pathName[128];
extern char parameter[64];
extern char baseName[128];
extern char dirName[128];

extern char pathNameTokenized[128];
extern char *pathNameTokenPtrs[128];
extern int tokenCount;


int get_block(int fd, int blk, char *buf)
{
    lseek(fd, (long)(blk * BLOCK_SIZE), 0);
    return read(fd, buf, BLOCK_SIZE);  // return: -1 (error)
                                       //          0 (EOF)
}


int put_block(int dev, int blk, char* buf)
{
    lseek(dev, (long)(blk * BLOCK_SIZE), 0);
    return write(dev, buf, BLOCK_SIZE);  // return: -1 (error)
}


int token_path(char *pathname, char **token_ptrs)
{
    int tok_i;
    token_ptrs[0] = strtok(pathname, "/\n");

    for (tok_i = 0; token_ptrs[tok_i]; tok_i++)
    {
        token_ptrs[tok_i + 1] = strtok(NULL, "/\n");
    }
    return tok_i;
}


char* dir_name(char* pathname)
{
    char temp[128];
    bzero(temp, 128);
    strncpy(temp, pathname, strlen(pathname) + 1);
    strncpy(dirName, dirname(temp), strlen(temp) + 1);
    return dirName;
}


char* base_name(char* pathname)
{
    char temp[128];
    bzero(temp, 128);
    strncpy(temp, pathname, strlen(pathname) + 1);
    strncpy(baseName, basename(temp), strlen(temp) + 1);
    return baseName;
}


u32 getfileino(MINODE *pip, char* name)
{
    char buf[BLOCK_SIZE];
    int i;
    char replace;
    MINODE *mip;
    u32 ret;
    for (i = 0; i < 12 && (pip->INODE).i_block[i]; i++)
    {
        get_block(pip->dev, (pip->INODE).i_block[i], buf);
        dp = (DIR *) buf;
        while ((char *) dp < buf + BLOCK_SIZE && dp->rec_len)
        {
            replace = dp->name[dp->name_len];
            dp->name[dp->name_len] = '\0';
            //printf("looking at record '%s'\n", dp->name);
            if (0 == strcmp(dp->name, name))
            {
                ret = dp->inode;
                mip = iget(pip->dev, dp->inode);
                if (FILE_MODE != (MASK_MODE & (mip->INODE).i_mode))
                {
                    iput(mip);
                    ret = -1;
                }
                //else
                //{
                //    printf("returning '%s'\n\n", dp->name);
                //}
                dp->name[dp->name_len] = replace;
                return ret;
            }
            dp->name[dp->name_len] = replace;
            dp = (DIR *) (((char *) dp) + dp->rec_len);
        }
    }
    return -1;
}

u32 getino (int* dev, char* pathname)
{

    MINODE* mip;
    u32 ino;
    int i, workingDev;
    char* token;

    if ('/' == pathname[0])
    {
        ino = root->ino;
        *dev = root->dev;
    }
    else
    {
        ino = running->cwd->ino;
        *dev = running->cwd->dev;
    }

    strncpy(pathNameTokenized, pathname, strlen(pathname) + 1);
    tokenCount = token_path(pathNameTokenized, pathNameTokenPtrs);

    if (tokenCount == 0) return -1;

    for (i = 0; i < tokenCount; i++)
    {
        mip = iget(*dev, ino);
        ino = search(mip, pathNameTokenPtrs[i]);
        if (mip->mounted)
            *dev = mip->mountptr->dev;
        else
            *dev = mip->dev;
        if (-1 == ino)
            return -1;
    }

    return ino;
}


u32 search (MINODE* mip, char* name)
{
    int i, dev;
    char *cp;
    char buf[BLOCK_SIZE];
    char temp[128];
    u32 myino, parent;

    // MOUNT changes
    if (mip->mounted)
    {
        ip = &(mip->mountptr->mounted_inode->INODE);
        dev = mip->mountptr->dev;

        if (0 == strcmp(name, ".."))
        {
            findino(mip, &myino, &parent);
            return parent;
        }
    }
    else
    {
        ip = &(mip->INODE);
        dev = mip->dev;
    }

    for (i = 0; i < EXT2_NDIR_BLOCKS; i++)
    {
        if (0 == ip->i_block[i]) break;

        get_block(dev, ip->i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;

        //printf("i=%d i_block[%d]=%d\n\n", i, i, ip->i_block[i]);
        //printf("   i_number rec_len name_len   name\n");

        while (cp < (buf + BLOCK_SIZE))
        {
            strncpy(temp, dp->name, dp->name_len + 1);
            temp[dp->name_len] = 0;
            //printf("   %5d    %4d    %4d       %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

            if (0 == strcmp(name, temp))
            {
                //printf("filetype %d\n", dp->file_type);
                if (EXT2_FT_DIR != dp->file_type) return -1; // Ensure it is a DIR
                //printf("found %s : ino = %d\n", temp, dp->inode);
                return dp->inode;
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return -1;
}


u32 search2 (MINODE* mip, char* name)
{
    int i, dev;
    char *cp;
    char buf[BLOCK_SIZE];
    char temp[128];

    // MOUNT changes
    if (mip->mounted)
    {
        ip = &(mip->mountptr->mounted_inode->INODE);
        dev = mip->mountptr->dev;
    }
    else
    {
        ip = &(mip->INODE);
        dev = mip->dev;
    }

    for (i = 0; i < EXT2_NDIR_BLOCKS; i++)
    {
        if (0 == ip->i_block[i]) break;

        get_block(dev, ip->i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;

        //printf("i=%d i_block[%d]=%d\n\n", i, i, ip->i_block[i]);
        //printf("   i_number rec_len name_len   name\n");

        while (cp < (buf + BLOCK_SIZE))
        {
            strncpy(temp, dp->name, dp->name_len + 1);
            temp[dp->name_len] = 0;
            //printf("   %5d    %4d    %4d       %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

            if (0 == strcmp(name, temp))
            {
                //printf("filetype %d\n", dp->file_type);
                //if (EXT2_FT_DIR != dp->file_type) return -1; // Ensure it is a DIR
                //printf("found %s : ino = %d\n", temp, dp->inode);
                return dp->inode;
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return -1;
}

u32 getino2 (int* dev, char* pathname)
{

    MINODE* mip;
    u32 ino;
    int i, workingDev;
    char* token;

    if ('/' == pathname[0])
    {
        ino = root->ino;
        *dev = root->dev;
    }
    else
    {
        ino = running->cwd->ino;
        *dev = running->cwd->dev;
    }

    strncpy(pathNameTokenized, pathname, strlen(pathname) + 1);
    tokenCount = token_path(pathNameTokenized, pathNameTokenPtrs);

    if (tokenCount == 0) return -1;

    for (i = 0; i < tokenCount; i++)
    {
        mip = iget(*dev, ino);
        ino = search2(mip, pathNameTokenPtrs[i]);
        if (mip->mounted)
            *dev = mip->mountptr->dev;
        else
            *dev = mip->dev;
        if (-1 == ino)
            return -1;
    }

    return ino;
}

int is_exist (MINODE* mip, char* name)
{
    int i;
    char *cp;
    char buf[BLOCK_SIZE];
    char temp[128];

    ip = &(mip->INODE);

    for (i = 0; i < 12 && ip->i_block[i]; i++)
    {
        get_block(mip->dev, ip->i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;

        while (cp < (buf + BLOCK_SIZE))
        {
            if (0 == dp->rec_len) break;
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            if (0 == strcmp(name, temp))
            {
                return -1;
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return 0;
}

MINODE* iget (int dev, u32 ino)
{
    int i;
    int blk, offset;
    char buf[BLOCK_SIZE];

    // Ensure INODE is not already loaded
    // search minode[] for an entry with (dev,ino) AND refCount > 0;
    for (i = 0; i < NMINODES; i++)
    {
        if (minode[i].dev == dev && minode[i].ino == ino && minode[i].refCount > 0)
        {
            minode[i].refCount++;
            return &minode[i];
        }
    }

    // Get INODE of (dev, ino)
    blk = (ino - 1)/8 + INODEBLOCK;
    offset = (ino - 1) % 8;
    get_block(dev, blk, buf);
    ip = (INODE*)buf + offset;

    // not found, use a FREE minode[i] to load the INODE of (dev, ino)
    for (i = 0; i < NMINODES; i++)
    {
        // FREE minode
        if (minode[i].refCount == 0)
        {
            minode[i].INODE = *ip; // load INODE of (dev,ino)
            minode[i].dev = dev;
            minode[i].ino = ino;
            minode[i].refCount = 1;
            minode[i].dirty = 0;
            minode[i].mounted = 0;  // NEEDS TO BE DONE
            minode[i].mountptr = 0; // NEEDS TO BE DONE
            return &minode[i];
        }
    }

    return NULL; // minode is full!
}

void
igetparentandfile(int *dev, MINODE **pip, MINODE **mip, char *name)
{
    u32 pino;
    u32 mino;

    *pip = NULL;
    *mip = NULL;

    pino = getino(dev, dir_name(name));
    if ((u32) -1 == pino)
    {
        printf("Couldn't fine path '%s'\n", dir_name(name));
        return;
    }

    *pip = iget(*dev, pino);
    if (!(*pip))
    {
        *pip = NULL;
        *mip = NULL;
        return;
    }
    mino = getfileino(*pip, base_name(name));
    if ((u32) -1 == mino)
    {
        *mip = NULL;
        return;
    }
    *mip = iget(*dev, mino);
}

void iput (MINODE* mip)
{
    int blk, offset;
    char buf[BLOCK_SIZE];

    mip->refCount--;
    if (0 < mip->refCount) { return; }
    if (!mip->dirty) { return; }
    if (0 == mip->refCount && mip->dirty)
    {
        // write INODE back to disk by its (dev, ino)

        // Get INODE of (dev, ino)
        blk = (mip->ino - 1)/8 + INODEBLOCK;
        offset = (mip->ino - 1) % 8;

        get_block(mip->dev, blk, buf);
        INODE* ip_ = (INODE*)buf + offset;

        //*ip = mip->INODE;
        memcpy(ip_, &mip->INODE, sizeof(INODE));

        //lseek(mip->dev, (long)((blk * BLOCK_SIZE) + offset), 0);
        //write(mip->dev, mip->INODE, sizeof(INODE));
        put_block(mip->dev, blk, buf);
    }
}

int findmyname (MINODE* parent, u32 myino, char* myname)
{
    int i;
    char* cp;
    char buf[BLOCK_SIZE];

    ip = &(parent->INODE);

    for (i = 0; i < EXT2_NDIR_BLOCKS; i++)
    {
        if (0 == ip->i_block[i]) break;

        get_block(parent->dev, ip->i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;

        //printf("i=%d i_block[%d]=%d\n\n", i, i, ip->i_block[i]);
        //printf("   i_number rec_len name_len   name\n");

        while (cp < (buf + BLOCK_SIZE))
        {
            strncpy(myname, dp->name, dp->name_len + 1);
            myname[dp->name_len] = 0;
            //printf("   %5d    %4d    %4d       %s\n", dp->inode, dp->rec_len, dp->name_len, myname);

            if (dp->inode == myino)
            {
                //printf("found ino = %d\n", dp->inode);
                return dp->inode;
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return -1;
}

int findino (MINODE* mip, u32* myino, u32* parent)
{
    char *cp;
    char buf[BLOCK_SIZE];

    ip = &(mip->INODE);

    if (0 == ip->i_block[0]) return -1;
    get_block (mip->dev, ip->i_block[0], buf);
    dp = (DIR*)buf;
    cp = buf;

    *myino = dp->inode;
    cp += dp->rec_len;
    dp = (DIR*)cp;
    *parent = dp->inode;

    return 0;
}

void incFreeInodes(int dev)
{
    char buf[BLOCK_SIZE];

    // Fix inode count in super block.
    get_block(dev, SUPERBLOCK, buf);
    sp = (SUPER *) buf;
    sp->s_free_inodes_count++;
    put_block(dev, SUPERBLOCK, buf);

    // Fix inode count in group descriptor block.
    get_block(dev, GDBLOCK, buf);
    gp = (GD *) buf;
    gp->bg_free_inodes_count++;
    put_block(dev, GDBLOCK, buf);
}

void idealloc(int dev, u32 ino)
{
    int i;
    char buf[BLOCK_SIZE];

    // get inode bitmap block
    get_block(dev, IBITMAP, buf);      // assume Imap is block 4
    CLR_bit(buf, ino-1);         // assume you have clr_bit() function

    // write buf back
    put_block(dev, IBITMAP, buf);

    // update free inode count in SUPER and GD
    incFreeInodes(dev);         // assume you write this function
}

void incFreeBlocks(int dev)
{
    char buf[BLOCK_SIZE];

    // Fix block count in super block.
    get_block(dev, SUPERBLOCK, buf);
    sp = (SUPER *) buf;
    sp->s_free_blocks_count++;
    put_block(dev, SUPERBLOCK, buf);

    // Fix block count in group descriptor block.
    get_block(dev, GDBLOCK, buf);
    gp = (GD *) buf;
    gp->bg_free_blocks_count++;
    put_block(dev, GDBLOCK, buf);
}

void bdealloc(int dev, u32 blk)
{
    int i;
    char buf[BLOCK_SIZE];

    // get block bitmap block
    get_block(dev, BBITMAP, buf);      // assume Bmap is block 3
    CLR_bit(buf, blk-1);         // assume you have clr_bit() function

    // write buf back
    put_block(dev, BBITMAP, buf);

    // update free block count in SUPER and GD
    incFreeBlocks(dev);         // assume you write this function
}

void decFreeInodes(int dev)
{
    char buf[BLOCK_SIZE];

    // Fix inode count in super block.
    get_block(dev, SUPERBLOCK, buf);
    sp = (SUPER *) buf;
    sp->s_free_inodes_count--;
    put_block(dev, SUPERBLOCK, buf);

    // Fix inode count in group descriptor block.
    get_block(dev, GDBLOCK, buf);
    gp = (GD *) buf;
    gp->bg_free_inodes_count--;
    put_block(dev, GDBLOCK, buf);
}

int ialloc (int dev)
{
    int i;
    char buf[BLOCK_SIZE];
    u32 ninodes; // FIXME needs to be replaced from MOUNT struct ninodes

    get_block(dev, SUPERBLOCK, buf);
    sp = (SUPER*)buf;
    ninodes = sp->s_inodes_count;

    get_block(dev, IBITMAP, buf);
    for (i = 0; i < ninodes; i++)
    {
        if (TST_bit(buf, i) == 0)
        {
            SET_bit(buf, i);
            put_block(dev, IBITMAP, buf);

            decFreeInodes(dev);
            return (i + 1);
        }
    }
    printf("fs panic : out of INODES\n");
    return 0;
}

void decFreeBlocks(int dev)
{
    char buf[BLOCK_SIZE];

    // Fix block count in super block.
    get_block(dev, SUPERBLOCK, buf);
    sp = (SUPER *) buf;
    sp->s_free_blocks_count--;
    put_block(dev, SUPERBLOCK, buf);

    // Fix block count in group descriptor block.
    get_block(dev, GDBLOCK, buf);
    gp = (GD *) buf;
    gp->bg_free_blocks_count--;
    put_block(dev, GDBLOCK, buf);
}

int balloc (int dev)
{
    int i;
    char buf[BLOCK_SIZE];
    u32 nblocks; //FIXME needs to replaced from MOUNT struct bnodes

    get_block(dev, SUPERBLOCK, buf);
    sp = (SUPER*)buf;
    nblocks = sp->s_blocks_count;

    get_block(dev, BBITMAP, buf);
    for (i = 0; i < nblocks; i++)
    {
        if (TST_bit(buf, i) == 0)
        {
            SET_bit(buf, i);
            put_block(dev, BBITMAP, buf);

            decFreeBlocks(dev);
            return (i + 1);
        }
    }
    return 0;
}

int TST_bit (char buf[], int BIT)
{
    return buf[BIT/8] & (1 << (BIT%8));
}

int SET_bit (char buf[], int BIT)
{
    return buf[BIT/8] |= (1 << (BIT%8));
}

int CLR_bit (char buf[], int BIT)
{
    return buf[BIT/8] &= ~(1 << (BIT%8));
}

int
real_rec_len(int name_len)
{
    // Round the rec len up to the nearest multiple of 4. Add 8 for the other
    // fields.
    return (((name_len + 3) / 4) * 4) + 8;
}

int
put_rec(MINODE *pip, char *name, u32 ino)
{
    int new_rec_len;
    int i;
    int new_block;
    char replace;
    char buf[BLOCK_SIZE];
    new_rec_len = real_rec_len(strlen(name));

    for (i = 0; i < 12 && (pip->INODE).i_block[i]; i++)
    {
        get_block(pip->dev, (pip->INODE).i_block[i], buf);
        dp = (DIR *) buf;
        while ((char *) dp < (buf + BLOCK_SIZE) && dp->rec_len)
        {
            replace = dp->name[dp->name_len];
            dp->name[dp->name_len] = '\0';
            printf("looking at record '%s'\n", dp->name);
            dp->name[dp->name_len] = replace;
            printf("rec len is %i\n", dp->rec_len);
            printf("expected rec len is %i\n", real_rec_len(dp->name_len));
            if(dp->rec_len -real_rec_len(dp->name_len) >= new_rec_len)
            {
                if (dp->rec_len - real_rec_len(dp->name_len) >= new_rec_len)
                {
                    new_rec_len = dp->rec_len - real_rec_len(dp->name_len);
                    dp->rec_len = real_rec_len(dp->name_len);
                    dp = (DIR *) (((char *) dp) + dp->rec_len);
                    dp->rec_len = new_rec_len;
                    dp->name_len = strlen(name);
                    strncpy(dp->name, name, strlen(name));
                    dp->inode = ino;
                    put_block(pip->dev, (pip->INODE).i_block[i], buf);
                    return 0;
                }
                break;
            }
            dp = (DIR *) (((char*) dp) + dp->rec_len);
        }
    }
    if (i < 12)
    {
        new_block = balloc(pip->dev);
        (pip->INODE).i_block[i] = new_block;
        dp = (DIR *) buf;
        dp->rec_len = BLOCK_SIZE;
        dp->name_len = strlen(name);
        strncpy(dp->name, name, strlen(name));
        dp->inode = ino;
        put_block(pip->dev, (pip->INODE).i_block[i], buf);
        return 0;
    }
    return -1;
}

int
del_rec(MINODE *pip, char *name)
{
    int i;
    char buf[BLOCK_SIZE];
    int new_block;
    char replace;
    int shift = 0;
    DIR *prev_dp;

    for (i = 0; i < 12 && (pip->INODE).i_block[i]; i++)
    {
        get_block(pip->dev, (pip->INODE).i_block[i], buf);
        shift = 0;
        dp = (DIR *) buf;
        while ((char *) dp < (buf + BLOCK_SIZE) && dp->rec_len)
        {
            replace = dp->name[dp->name_len];
            dp->name[dp->name_len] = '\0';
            if (shift)
            {
                if (((char *) dp) + dp->rec_len >= buf + BLOCK_SIZE - 1)
                {
                    printf("found final rec, increasing rec_len by %i bytes\n", shift);
                    dp->rec_len += shift;
                }
                // We already deleted the record and we're shifting the rest
                // down.
                printf("Shifting record for '%s' down by %i bytes.\n", dp->name, shift);
                dp->name[dp->name_len] = replace;
                memmove(((char *) dp) - shift, dp, dp->rec_len);
            }
            else if (0 == strcmp(name, dp->name))
            {
                printf("We found a rec structure with name '%s'\n", dp->name);
                dp->name[dp->name_len] = replace;
                if (BLOCK_SIZE == dp->rec_len)
                {
                    printf("It's the only record on this block. Just deleting the block.\n");
                    // This is the only dir record in the block. Just bdealloc
                    // it.
                    bdealloc(pip->dev, (pip->INODE).i_block[i]);
                    (pip->INODE).i_block[i] = 0;
                    (pip->INODE).i_size -= BLOCK_SIZE;
                    return 0;
                }
                else if (((char *) dp) + dp->rec_len >= buf + BLOCK_SIZE)
                {
                    // No need to shift stuff, just extend the previous rec_len
                    // to the end of the block.
                    prev_dp->rec_len += dp->rec_len;
                }
                else
                {
                    // Shift all the directory records down.
                    shift = dp->rec_len;
                    printf("It's not the only record on this block. Shifting other records by %i.\n", shift);
                }
            }
            else
            {
                dp->name[dp->name_len] = replace;
            }
            prev_dp = dp;
            dp = (DIR *) (((char *) dp) + dp->rec_len);
        }
        put_block(pip->dev, (pip->INODE).i_block[i], buf);
    }
    return 0;
}

OFT* falloc()
{
    int i;
    for (i = 0; i < NOFT; i++)
    {
        if (0 == oft[i].refCount)
        {
            return &(oft[i]);
        }
    }
    err_printf("panic : open file table is full!\n");
    return NULL;
}

void err_printf(char* msg)
{
    printf("\033[31m%s\033[0m", msg);
}
