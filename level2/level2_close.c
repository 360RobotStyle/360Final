#include "../filesystem.h"
#include "../util.h"

void
do_close()
{
    int fd;

    if (0 == strcmp(pathName, ""))
    {
        printf("close : missing file descriptor\n");
        return;
    }
    fd = atoi(pathName);
    if ((!running->fd[fd]) || !(running->fd[fd]->refCount))
    {
        printf("File '%s' is not open.\n", pathName);
    }
    else
    {
        (running->fd[fd])->refCount--;
        iput(running->fd[fd]->inodeptr); // mip has a matching ref count.
        printf("Closed '%s'. File has %i open file descriptors.\n", pathName,
                (running->fd[fd])->refCount);
        if (!(running->fd[fd]->refCount))
        {
            running->fd[fd] = 0;
        }
        return;
    }
}
