#include "util.h"

// util.h
//extern INODE myinode;
extern PROC proc[2];
extern PROC* running;
extern PROC* readyQueue;
extern MINODE minode[NMINODES];
extern MINODE* root;

extern char pathName[256];
extern char parameter[256];
extern char baseName[128];
extern char dirName[128];

extern char pathNameTokenized[256];
extern char *pathNameTokenPtrs[256];
extern int tokenCount;

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

char* dir_name(char* pathname)
{
    char temp[256];
    bzero(temp, 256);
    strncpy(temp, pathname, strlen(pathname));
    strncpy(dirName, dirname(temp), strlen(temp));
    return dirName;
}

char* base_name(char* pathname)
{
    char temp[256];
    bzero(temp, 256);
    strncpy(temp, pathname, strlen(pathname));
    strncpy(baseName, basename(temp), strlen(temp));
    return baseName;
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

    strncpy(pathNameTokenized, pathname, strlen(pathname));
    tokenCount = token_path(pathNameTokenized, pathNameTokenPtrs);

    if (tokenCount == 0) return -1;

    for (i = 0; i < tokenCount; i++)
    {
        mip = iget(*dev, ino);
        //printf("=============================================\n");
        //printf("i=%d name[%d]=%s\n", i, i, pathNameTokenPtrs[i]);
        //printf("search for %s in %x\n", pathNameTokenPtrs[i], (u32)&(mip->inode));
        ino = search(mip, pathNameTokenPtrs[i]);
        *dev = mip->dev;
        if ((ino == 0) || ((dp->file_type != EXT2_FT_DIR) && ((i + 1) < tokenCount)))
            return -1;
    }

    return dp->inode;
}


u32 search (MINODE* mip, char* name)
{
    int i;
    char *cp;
    char buf[BLOCK_SIZE];
    char temp[128];

    ip = &(mip->INODE);

    for (i = 0; i < EXT2_NDIR_BLOCKS; i++)
    {
        if (0 == ip->i_block[i]) break;

        get_block(mip->dev, ip->i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;

        printf("i=%d i_block[%d]=%d\n\n", i, i, ip->i_block[i]);
        printf("   i_number rec_len name_len   name\n");

        while (cp < (buf + BLOCK_SIZE))
        {
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            printf("   %5d    %4d    %4d       %s\n", dp->inode, dp->rec_len, dp->name_len, temp);

            if (0 == strcmp(name, temp))
            {
                printf("found %s : ino = %d\n", temp, dp->inode);
                return dp->inode;
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return -1;
}


MINODE* iget (int dev, unsigned long ino)
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
