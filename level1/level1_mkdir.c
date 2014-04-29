#include "../filesystem.h"
#include "../util.h"


static int
my_mkdir(MINODE* pip, char* name)
{
    int i;
    MINODE* mip;
    u32 inumber, bnumber;
    char buf[BLOCK_SIZE];
    char* cp;

    // allocate an inode and disk block for the new dir
    inumber = ialloc(pip->dev);
    bnumber = balloc(pip->dev);

    // to load INODE into a minode[],
    mip = iget(pip->dev, inumber);

    // write contents into the intended INODE in memory
    mip->INODE.i_mode = 0x41ED;
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->INODE.i_size = 1024;

    mip->INODE.i_links_count = 2;
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);

    mip->INODE.i_blocks = 2;
    mip->dirty = 1;

    for (i = 0; i < 15; i++)
        mip->INODE.i_block[i] = 0;
    mip->INODE.i_block[0] = bnumber;

    iput(mip);

    // write . and .. entries into a buf[] of BLOCK_SIZE
    // write buf[] to the disk block allocated to this dir
    dp = (DIR*)buf;
    dp->inode = inumber;
    strncpy(dp->name, ".", 1);
    dp->name_len = 1;
    dp->rec_len = 12;

    cp = buf + 12;
    dp = (DIR*)cp;

    dp->inode = pip->ino;
    dp->name_len = 2;
    strncpy(dp->name, "..", 2);
    dp->rec_len = BLOCK_SIZE - 12;

    put_block(pip->dev, bnumber, buf);

    // enter name into parent's directory
/*
    // read parent's data block into buf[]
    need_length = 4 * ((8 + name_len + 3) / 4);
    ideal_length = 4 * ((8 + name_len + 3) / 4);

    if (rec_len - ideal_length >= need_length)
    {
        // enter the new entry as the LAST entry and trim the previous
        // entry to its ideal length
    }
    else
    {
        // allocate a new data block
        // enter the new entry as the first entry in the new data block
    }
*/
    // FIXME write parent's data block back to disk

    // FIXME increment parent's inode's link count by 1
    // touch its atime
    // mark it DIRTY
    iput(pip);
}


static u32
isExist (MINODE* mip, char* name)
{
    int i;
    char *cp;
    char buf[BLOCK_SIZE];
    char temp[128];

    ip = &(mip->INODE);

    for (i = 0; i < EXT2_NDIR_BLOCKS; i++)
    {
        if (0 == ip->i_block[i]) break;

        get_block(mip->dev, ip->i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;

        while (cp < (buf + BLOCK_SIZE))
        {
            strncpy(temp, dp->name, dp->name_len + 1);
            temp[dp->name_len] = 0;

            if (0 == strcmp(name, temp))
            {
                return -1;
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return 0;
}


void
make_dir()
{
    int dev;
    u32 ino;
    char* parent;
    char* child;
    MINODE* pip;

    if ('/' == pathName[0])
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    parent = dir_name(pathName);
    child = base_name(pathName);

    ino = getino(&dev, parent);
    pip = iget(dev, ino);

    // verify parent INODE is a DIR and child does not exist in the parent dir
    if ((pip->INODE.i_mode & 0040000) == 0040000 && -1 == isExist(pip, child))
    {
        my_mkdir(pip, child);
    }
}
