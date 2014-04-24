#include "util.h"

// util.h
//extern INODE myinode;
extern PROC proc[2];
extern PROC* running;
extern PROC* readyQueue;
extern MINODE minode[NMINODES];
extern MINODE* root;

extern char pathname[256];
extern char parameter[256];
extern char baseName[128];
extern char dirName[128];
extern COMPONENTS pathTokens;

int get_block (int fd, int blk, char *buf)
{
    lseek(fd, (long)(blk * BLOCK_SIZE), 0);
    return read(fd, buf, BLOCK_SIZE);  // return: -1 (error)
                                       //          0 (EOF)
}

put_block (int dev, int blk, char* buf)
{

}

char* dir_name()
{
    char temp[256];

    strcpy(temp, pathname);
    strcpy(dirName, dirname(temp));
    return dirName;
}

char* base_name()
{
    char temp[256];

    strcpy(temp, pathname);
    strcpy(baseName, basename(temp));
    return baseName;
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
