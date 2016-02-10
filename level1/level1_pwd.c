#include "../filesystem.h"
#include "../util.h"

static void
pwd_helper(MINODE* mip, char* path_buf)
{
    MINODE* pip;
    u32 myino;
    u32 parentino;
    char dirname_buf[64];
    dirname_buf[0] = '\0';

    if (mip != root)
    {
        findino(mip, &myino, &parentino);
        // FIXME using the mip->dev will not work if the parent directory is on
        // a different drive.
        pip = iget(mip->dev, parentino);
        pwd_helper(pip, path_buf);
        findmyname(pip, myino, dirname_buf);
        strcat(path_buf, "/");
        strcat(path_buf, dirname_buf);
        iput(pip);
    }
}

void
do_pwd()
{
    char path_buf[128];

    if (running->cwd == root)
    {
        printf("/\n");
    }
    else
    {
        path_buf[0] = '\0';
        pwd_helper(running->cwd, path_buf);
        printf("%s\n", path_buf);
    }
}
