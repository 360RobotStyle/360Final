#include "../filesystem.h"
#include "../util.h"

void
touch_file()
{
    u32 ino;
    int dev;
    MINODE* mip;

    if (0 == strcmp(pathName, ""))
    {
        printf("touch : missing argument\n");
        return;
    }
    else
    {
        ino = getino(&dev, pathName);
        if ((u32)-1 == ino)
        {
            ino = getino(&dev, dir_name(pathName));
            mip = iget(dev, ino);
            ino = getfileino(mip, base_name(pathName));
            iput(mip);
            mip = iget(dev, ino);
        }
        else
            mip = iget(dev, ino);

        mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
        mip->dirty = 1;

        iput(mip);
    }
}
