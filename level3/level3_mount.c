#include "../filesystem.h"
#include "../util.h"

void
do_mount()
{

    int i;
    int dev;
    int fd;
    int mino;
    MINODE *mip;
    char buf[BLOCK_SIZE];
    MOUNT *mnt;
// mount()    /*  Usage: mount filesys mount_point OR mount */
// 
// 1. Ask for filesys (a pathname) and mount_point (a pathname also).
// If mount with no parameters: display current mounted filesystems.
//

    if (0 == strcmp(pathName, ""))
    {
        // Display mount points and exit.
        printf("\n*****************************************************\n");
        printf("%20s   fd %20s\n", "device name", "mount point");
        printf("*****************************************************\n");
        for (i = 0; i < NMOUNT; i++)
        {
            if (mount[i].dev)
            {
                printf("%20s %4i %20s\n", 
                        mount[i].image_name, mount[i].dev, mount[i].mount_name);
            }
        }
        return;
    }

    if (0 == strcmp(parameter, "") || 
            0 == strcmp(parameter, ".") ||
            0 == strcmp(parameter, ".."))
    {
        printf("mount : Invalid mount point '%s'\n", parameter);
        return;
    }
    mino = getino(&dev, parameter);
    if (-1 == mino)
    {
        printf("mount : Invalid mount point '%s'\n", parameter);
        return;
    }
    mip = iget(dev, mino);
    if (0x4000 != (0x4000 & (mip->INODE).i_mode))
    {
        iput(mip);
        printf("mount : Invalid mount point '%s'\n", parameter);
        return;
    }

// 2. Check whether filesys is already mounted: 
// (you may store the name of mounted filesys in the MOUNT table entry). 
// If already mounted, reject;
// else: allocate a free MOUNT table entry (whose dev == 0 means FREE).

    for (i = 0; i < NMOUNT; i++)
    {
        if (mount[i].dev && 0 == strcmp(mount[i].image_name, pathName))
        {
            printf("mount : Image '%s' is already mounted\n", pathName);
            iput(mip);
            return;
        }
    }
    fd = open(pathName, O_RDWR);
    if (fd < 0)
    {
        printf("mount : can't open '%s'\n", pathName);
        iput(mip);
        return;
    }
    mnt = oalloc(fd);

    // check InodesBeginBlock
    get_block(fd, GDBLOCK, buf);
    gp = (GD*)buf;
    if (INODEBLOCK != gp->bg_inode_table)
    {
        err_printf("error: inode begin block\n");
        odealloc(fd);
        iput(mip);
        return;
    }

    // read SUPER block to verify it's an EXT2 FS
    get_block(fd, SUPERBLOCK, buf);
    sp = (SUPER*)buf;
    if (SUPER_MAGIC != sp->s_magic)
    {
        err_printf("error: not ext2 filesystem\n");
        odealloc(fd);
        iput(mip);
        return;
    }
    printf("nblocks=%d  bfree=%d   ninodes=%d  ifree=%d\n",
                sp->s_blocks_count, sp->s_free_blocks_count,
                sp->s_inodes_count, sp->s_free_inodes_count);

    mip->mounted = 1;
    mip->mountptr = mnt;

    mnt->mounted_inode = iget(fd, ROOT_INODE);

    printf("mounted root\n");
    mnt->dev = fd;
    mnt->ninodes = sp->s_inodes_count;
    mnt->nblocks = sp->s_blocks_count;
    strncpy(mnt->image_name, pathName, 256);
    strncpy(mnt->mount_name, parameter, 256);

    printf("mount : success\n");
    // FIXME verify mount point.



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
