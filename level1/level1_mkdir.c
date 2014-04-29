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
    int need_length, ideal_length, rec_len;

    // allocate an inode and disk block for the new dir
    inumber = ialloc(pip->dev);
    bnumber = balloc(pip->dev);
    // to load INODE into a minode[],
    mip = iget(pip->dev, inumber);

    // write contents into the intended INODE in memory
    mip->INODE.i_mode = D_DIR_MODE;
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
    need_length = 4 * ((8 + strlen(name) + 3) / 4);

    // read parent's data block into buf[]
    ip = &(pip->INODE);
    for (i = 0; i < 12 &&  ip->i_block[i]; i++)
    {
        get_block(pip->dev, ip->i_block[i], buf);
        cp = buf;
        dp = (DIR*)buf;
        while (cp < (buf + BLOCK_SIZE))
        {
            dp = (DIR*)cp;
            cp += dp->rec_len;
        }
        cp -= dp->rec_len;

        // dp now points to the LAST entry
        ideal_length = 4 * ((8 + dp->name_len + 3) / 4);
        rec_len = dp->rec_len;
        if (rec_len - ideal_length >= need_length)
        {
            // enter the new entry as the LAST entry and trim the previous
            // entry to its ideal length
            dp->rec_len = ideal_length;
            cp += dp->rec_len;
            dp = (DIR*)cp;
            dp->inode = inumber;
            printf("PIP ino %u\n", dp->inode);
            dp->name_len = strlen(name);
            strncpy(dp->name, name, dp->name_len);
            dp->rec_len = rec_len - ideal_length;
            put_block(pip->dev, ip->i_block[i], buf);
            break;
        }
        else
        {
            if (0 == ip->i_block[i + 1])
            {
                // allocate a new data block
                // enter the new entry as the first entry in the new data block
                get_block(pip->dev, ip->i_block[i + 1], buf);
                dp = (DIR*)buf;
                dp->inode = inumber;
                strncpy(dp->name, ".", 1);
                dp->name_len = 1;
                dp->rec_len = BLOCK_SIZE;
                put_block(pip->dev, ip->i_block[i + 1], buf);
                break;
            }
        }
    }

    pip->refCount++;
    pip->INODE.i_atime = time(0L);
    pip->dirty = 1;

    iput(pip);
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

    if (0 == strcmp(child, "."))
    {
        printf("mkdir : provide a directory name\n");
        return;
    }
    ino = getino(&dev, parent);
    pip = iget(dev, ino);
    // verify parent INODE is a DIR and child does not exist in the parent dir
    if ((0040000 == (pip->INODE.i_mode & 0040000)) && (0 == is_exist(pip, child)))
    {
        my_mkdir(pip, child);
    }
}
