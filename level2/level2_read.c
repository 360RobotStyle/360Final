#include "../filesystem.h"
#include "../util.h"

static void
myread(int fd, char* buf, int nbytes)
{

}

void
read_file()
{
    int fd;
    int i;
    int nbytes;
    char buf[BLOCK_SIZE];

    if (1 == strlen(pathName) && pathName[0] >= '0' && pathName[0] <= '9')
    {
        if (0 == strlen(parameter))
        {
            printf("read : bad nbytes\n");
            return;
        }

        for (i = 0; i < strlen(parameter); i++)
        {
            if (!isdigit(parameter[i]))
            {
                printf("read : invalid bytes\n");
                return;
            }
        }

        // 1) get fd & nbytes
        fd = atoi(pathName);
        nbytes = atoi(parameter);

        // 2) verify fd is indeed opened for R or RW mode
        if ((0 != running->fd[fd]) && (0 != running->fd[fd]->refCount) &&
            (0 == running->fd[fd]->mode || 1 == running->fd[fd]->mode))
        {
            myread(fd, buf, nbytes);
        }
        else
        {
            printf("read : fd = %d is NOT FOR READ\n", fd);
        }
    }
    else
    {
        printf("read : ensure correct fd is provided\n");
    }
}
