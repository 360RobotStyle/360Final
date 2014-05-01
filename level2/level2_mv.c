#include "../filesystem.h"
#include "../util.h"

void
do_mv()
{
    MINODE *pip; // Destination
    MINODE *mip; // Destination
    MINODE *sip; // Destination
    char buf[BLOCK_SIZE];
    u32 pino;
    u32 mino;
    u32 src_ino;
    int dev;
    int i;
    u32 ino;

    // We're using pathName as first file path and parameter as second file path.

    if (dir_name(pathName))
    {
        // We're given some directories to traverse.
        pino = getino(&dev, dir_name(pathName));
    }
    else
    {
        // We're not given directories to traverse.
        pino = running->cwd->ino;
        dev = running->cwd->dev;
    }
    if (-1 == pino)
    {
        printf("Error: Could not find folder '%s'\n", dir_name(pathName));
        return;
    }
    pip = iget(dev, pino);
    mino = getfileino(pip, base_name(pathName));
    iput(pip);

    if (-1 == mino)
    {
        printf("Error: Could not find '%s'\n", pathName);
        return;
    }
    mip = iget(dev, mino); // Finally have the minode for the dest file.

    if (FILE_MODE != (MASK_MODE & (mip->INODE).i_mode))
    {
        printf("Error: %s is not a file. Has mode %x\n", pathName, (mip->INODE).i_mode);
        iput(mip);
        return;
    }
    else if (dir_name(parameter) && -1 == getino(&dev, dir_name(parameter)))
    {
    // Couldn't find the inode for the dir folder we were looking for.
        printf("Error: Couldn't find host folder '%s'\n", dir_name(parameter));
        iput(mip);
        return;
    }
    else if (-1 != getino(&dev, parameter))
    {
        // something already exists where we want to make the link.
        printf("Error: '%s' already exists\n", parameter);
        iput(mip);
        return;
    }
    if (!(dir_name(parameter)))
    {
        sip = iget(mip->dev, getino(&dev, ".")); // This is actually the parent of the link src.
    }
    else
    {
        sip = iget(mip->dev, getino(&dev, dir_name(parameter))); // This is actually the parent of the link src.
    }
    if (-1 == put_rec(sip, base_name(parameter), mino))
    {
        printf("Didn't succeed in placing '%s' record\n", parameter);
        iput(mip);
        iput(sip);
        return;
    }
    (mip->INODE).i_links_count++;
    sip->dirty = 1;
    mip->dirty = 1;
    iput(sip);
    iput(mip);

    //strcpy(pathName, parameter);
    do_unlink();
}
