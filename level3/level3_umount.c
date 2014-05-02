#include "../filesystem.h"
#include "../util.h"

void
do_umount()
{
    int i;
    MOUNT* umnt = 0;

    // 1) Search the MOUNT table to check filesys is indeed mounted
    // index 0 is ALLOTED for ROOT mount
    for (i = 1; i < NMOUNT; ++i)
    {
        if (mount[i].dev && 0 == strcmp(mount[i].mount_name, pathName))
        {
            umnt = &mount[i];
            break;
        }
    }

    if (!umnt)
    {
        printf("umount: filesys does not exist\n");
        return;
    }

    // 2) Check whether any file is still active in themounted filesys;
    //    e.g. someone's CWD or opened files are still there,
    //    if so, the mounted filesys is BUSY ==> cannot be unmounted yet
    // HOW to check?  ANS: by checking all minode[].dev
    for (i = 0; i < NMINODES; i++)
    {
        if (minode[i].refCount && minode[i].dev == umnt->dev)
        {
            printf("umount : filesys is busy, cannot umount\n");
            return;
        }
    }

    // 3) Find the mount_point's inode (which should be in memory while it's mounted
    //    on). Reset it to "not mounted"; then
    //         iput() the minode. (because it was iget()ed during mounting)
    for (i = 0; i < NMINODES; i++)
    {
        if (minode[i].refCount && minode[i].mountptr->dev == umnt->dev)
        {
            minode[i].mountptr = 0;
            iput(&minode[i]);
            umnt->mounted_inode = 0;
            umnt->dev = 0;
            break;
        }
    }

    // 4) return (0);
}
