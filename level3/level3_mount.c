#include "../filesystem.h"
#include "../util.h"

void
do_mount()
{

    int i;
    int dev;
    int mino;
// mount()    /*  Usage: mount filesys mount_point OR mount */
// 
// 1. Ask for filesys (a pathname) and mount_point (a pathname also).
// If mount with no parameters: display current mounted filesystems.
//
    mino = getino(&dev, ".");
    printf("my dev is %i\n", dev);


    if (0 == strcmp(pathName, ""))
    {
        // TODO Display mount points and exit.
        printf("should display mount points\n");
    }

// 2. Check whether filesys is already mounted: 
// (you may store the name of mounted filesys in the MOUNT table entry). 
// If already mounted, reject;
// else: allocate a free MOUNT table entry (whose dev == 0 means FREE).

    for (i = 0; i < NMOUNT; i++)
    {
        
    }
// 
// 3. open filesys for RW; use its fd number as the new DEV;
// Check whether it's an EXT2 file system: if not, reject.
// 
// 4. find the ino, and then the minode of mount_point:
// call  ino  = get_ino(&dev, pathname);  to get ino:
// call  mip  = iget(dev, ino);           to load its inode into memory;
// 
// 5. Check mount_point is a DIR.  
// Check mount_point is NOT busy (e.g. can't be someone's CWD)
// 
// 6. Record new DEV in the MOUNT table entry;
// 
// (For convenience, store the filesys name in the Mount table, and also
// store its ninodes, nblocks, etc. for quick reference)
// 
// 7. Mark mount_point's minode as being mounted on and let it point at the
// MOUNT table entry, which points back to the mount_point minode.
// 
// . return 0;

}
