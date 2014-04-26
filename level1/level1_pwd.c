#include "../filesystem.h"
#include "../util.h"

static void
pwd_helper(MINODE* mip, char* path_buf)
{
    MINODE* pip;
    int myino;
    int parentino;
    char dirname_buf[64];
    dirname_buf[0] = '\0';

    if (mip != root)
    {
        findino(mip, &myino, &parentino);
        pip = iget(mip->dev, parentino);
        pwd_helper(pip, path_buf);
        findmyname(pip, myino, dirname_buf);
        strcat(path_buf, "/");
        strcat(path_buf, dirname_buf);
        iput(pip);
    }
}

void
pwd()
{
    char path_buf[128];
    buf[0] = '\0';
    pwd_helper(running->cwd, path_buf);
    printf("%s\n", path_buf);
}
