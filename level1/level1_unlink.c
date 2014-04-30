#include "../filesystem.h"
#include "../util.h"

void
do_unlink()
{
    MINODE *pip; // Destination
    MINODE *mip; // Destination
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
    if (1 >= (mip->INODE).i_links_count)
    {
        iput(pip);
        iput(mip);
        printf("Destroying last link to '%s'. Calling rm instead.\n", pathName);
        do_rm();
        return;
    }
    else if (FILE_MODE != (MASK_MODE & (mip->INODE).i_mode))
    {
        printf("Error: %s is not a file. Has mode %x\n", pathName, (mip->INODE).i_mode);
        iput(mip);
        return;
    }
    (mip->INODE).i_links_count--;
    del_rec(pip, base_name(pathName));
    pip->dirty = 1;
    mip->dirty = 1;
    iput(mip);
    iput(pip);
}
