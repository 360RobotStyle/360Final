#include "../filesystem.h"
#include "../util.h"

#define EXT2_S_IRUSR   0x0100  // user read
#define EXT2_S_IWUSR   0x0080  // user write
#define EXT2_S_IXUSR   0x0040  // user execute
#define EXT2_S_IRGRP   0x0020  // group read
#define EXT2_S_IWGRP   0x0010  // group write
#define EXT2_S_IXGRP   0x0008  // group execute
#define EXT2_S_IROTH   0x0004  // others read
#define EXT2_S_IWOTH   0x0002  // others write
#define EXT2_S_IXOTH   0x0001  // others execute

void
chmod_file()
{
    u32 ino;
    int dev;
    MINODE* mip;
    int i;
    __u16 modes[3] = {0,0,0};

    if (0 == strcmp(pathName, "") || 0 == strcmp(parameter, ""))
    {
        printf("chmod : missing argument and/or parameter\n");
        return;
    }
    else if ('0' == parameter[0] && 4 == strlen(parameter))
    {
        // octal form
        for (i = 1; i < 4; i++)
        {
            if (parameter[i] < '0' || parameter[i] > '7')
            {
                printf("chmod : invalid mode %s\n", parameter);
                return;
            }
        }
        // user
        if      ('7' == parameter[1]) modes[0] = EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IXUSR;
        else if ('6' == parameter[1]) modes[0] = EXT2_S_IRUSR | EXT2_S_IWUSR;
        else if ('5' == parameter[1]) modes[0] = EXT2_S_IRUSR | EXT2_S_IXUSR;
        else if ('4' == parameter[1]) modes[0] = EXT2_S_IRUSR;
        else if ('3' == parameter[1]) modes[0] = EXT2_S_IWUSR | EXT2_S_IXUSR;
        else if ('2' == parameter[1]) modes[0] = EXT2_S_IWUSR;
        else if ('1' == parameter[1]) modes[0] = EXT2_S_IXUSR;
        else if ('0' == parameter[1]) modes[0] = 0;
        // group
        if      ('7' == parameter[2]) modes[1] = EXT2_S_IRGRP | EXT2_S_IWGRP | EXT2_S_IXGRP;
        else if ('6' == parameter[2]) modes[1] = EXT2_S_IRGRP | EXT2_S_IWGRP;
        else if ('5' == parameter[2]) modes[1] = EXT2_S_IRGRP | EXT2_S_IXGRP;
        else if ('4' == parameter[2]) modes[1] = EXT2_S_IRGRP;
        else if ('3' == parameter[2]) modes[1] = EXT2_S_IWGRP | EXT2_S_IXGRP;
        else if ('2' == parameter[2]) modes[1] = EXT2_S_IWGRP;
        else if ('1' == parameter[2]) modes[1] = EXT2_S_IXGRP;
        else if ('0' == parameter[2]) modes[1] = 0;
        // others
        if      ('7' == parameter[3]) modes[2] = EXT2_S_IROTH | EXT2_S_IWOTH | EXT2_S_IXOTH;
        else if ('6' == parameter[3]) modes[2] = EXT2_S_IROTH | EXT2_S_IWOTH;
        else if ('5' == parameter[3]) modes[2] = EXT2_S_IROTH | EXT2_S_IXOTH;
        else if ('4' == parameter[3]) modes[2] = EXT2_S_IROTH;
        else if ('3' == parameter[3]) modes[2] = EXT2_S_IWOTH | EXT2_S_IXOTH;
        else if ('2' == parameter[3]) modes[2] = EXT2_S_IWOTH;
        else if ('1' == parameter[3]) modes[2] = EXT2_S_IXOTH;
        else if ('0' == parameter[3]) modes[2] = 0;

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

        if ((mip->INODE.i_mode & 0100000) == 0100000) mip->INODE.i_mode = 0100000;
        if ((mip->INODE.i_mode & 0040000) == 0040000) mip->INODE.i_mode = 0040000;
        if ((mip->INODE.i_mode & 0120000) == 0120000) mip->INODE.i_mode = 0120000;

        mip->INODE.i_mode |= modes[0] | modes[1] | modes[2];

        mip->dirty = 1;

        iput(mip);
    }
    else if (('+' == parameter[0] || '-' == parameter[0]) && 2 == strlen(parameter))
    {
        // fancier format
        if      ('w' == parameter[1]) modes[0] = EXT2_S_IWUSR | EXT2_S_IWGRP | EXT2_S_IWOTH;
        else if ('r' == parameter[1]) modes[0] = EXT2_S_IRUSR | EXT2_S_IRGRP | EXT2_S_IROTH;
        else if ('x' == parameter[1]) modes[0] = EXT2_S_IXUSR | EXT2_S_IXGRP | EXT2_S_IXOTH;
        else
        {
            printf("chmod : unknown parameter\n");
            return;
        }

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

        if ('+' == parameter[0])
            mip->INODE.i_mode |= modes[0];
        else
            mip->INODE.i_mode &= ~modes[0];
        mip->dirty = 1;

        iput(mip);
    }
    else
    {
        printf("chmod : invalid parameter\n");
    }


}
