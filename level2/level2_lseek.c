#include "../filesystem.h"
#include "../util.h"

void
do_lseek()
{
    int fd;
    u32 offset;
    printf("getting to lseek\n");

    if (0 == strcmp(pathName, ""))
    {
        printf("close : missing file descriptor\n");
        return;
    }
    if (0 == strcmp(parameter, ""))
    {
        printf("lseek : missing offset\n");
        return;
    }

    fd = atoi(pathName);
    offset = (u32) atoi(parameter);

    if ((!running->fd[fd]) || !(running->fd[fd]->refCount))
    {
        running->fd[fd] = 0;
        printf("File '%s' is not open.\n", pathName);
    }
    else if (offset > ((running->fd[fd])->inodeptr->INODE).i_size)
    {
        printf("Offset exceeds file boundary %i\n", (int) ((running->fd[fd])->inodeptr->INODE).i_size);
    }
    else
    {
        printf("Moving offset from %i to %i\n", (int) (running->fd[fd])->offset, (int) offset);
        (running->fd[fd])->offset = offset;
    }
}

