#include "../filesystem.h"
#include "../util.h"

void
quit()
{
    int i;

    // ensure that before quitting, all dirty inode are saved
    for (i = 0; i < NMINODES; i++)
    {
        if (minode[i].refCount >= 1)
        {
            minode[i]. refCount = 1;
            iput(&minode[i]);
        }
    }
    exit(0);
}
