#include "../filesystem.h"
#include "../util.h"

void
chgrp_file()
{
    u32 ino;
    int dev;
    MINODE* mip;

    if (0 == strcmp(pathName, "") || 0 == strcmp(parameter, ""))
    {
        printf("touch : missing argument\n");
        return;
    }
    else if (parameter[0] >= '0' && parameter[0] <= '1')
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

        // FIXME
        mip->INODE.i_gid = parameter[0]=='0' ? 0 : 1;
        mip->dirty = 1;

        iput(mip);
    }
}
