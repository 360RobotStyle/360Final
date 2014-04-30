#include "../filesystem.h"
#include "../util.h"

void
pfd()
{
    if (1)
    {
        printf("==================   pid = %d   ===================\n", running->pid);
        printf("fd    mde   count  offset  [dev,ino]    filename\n"
               "--   -----  -----  ------  ---------   ------------\n");
        printf("\033[33m%2d\033[0m   %s   %03d    %04d    [%2d, %2d]    %s\n", 0, "WRITE", 1, 0, 3, 21, "hello.c");
        printf("==================================================\n");
    }
    else
    {
        printf("no opened files\n");
    }
}
