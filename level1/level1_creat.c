#include "../filesystem.h"
#include "../util.h"

static void
my_creat(MINODE* pip, char* name)
{
    int i;
    MINODE* mip;
    u32 inumber, bnumber;
    char buf[BLOCK_SIZE];
    char* cp;
    int need_length, ideal_length, rec_len;

    // allocate an inode and disk block for the new dir
    inumber = ialloc(pip->dev);

    // to load INODE into a minode[],
    mip = iget(pip->dev, inumber);
    //printf("Inumber %d Bnumber %d\n", inumber, bnumber);
    // write contents into the intended INODE in memory
    mip->INODE.i_mode = D_FILE_MODE;
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->INODE.i_size = 0;

    mip->INODE.i_links_count = 1;
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);

    mip->INODE.i_blocks = 0;
    for (i = 0; i < 15; i++)
        mip->INODE.i_block[i] = 0;
    mip->dirty = 1;

    iput(mip);

    // enter name into parent's directory
    need_length = 4 * ((8 + strlen(name) + 3) / 4);

    //put_rec(pip, name, inumber);
    for (i = 0; i < 12 && (pip->INODE).i_block[i]; i++)
    {
        // read parent's data block into buf[]
        get_block(pip->dev, (pip->INODE).i_block[i], buf);
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
            dp->name_len = strlen(name);
            dp->file_type = EXT2_FT_REG_FILE;
            strncpy(dp->name, name, dp->name_len);
            dp->rec_len = rec_len - ideal_length;
            put_block(pip->dev, (pip->INODE).i_block[i], buf);
            break;
        }
        else
        {
            if (0 == (pip->INODE).i_block[i + 1])
            {
                // allocate a new data block
                // enter the new entry as the first entry in the new data block
                get_block(pip->dev, (pip->INODE).i_block[i + 1], buf);
                dp = (DIR*)buf;
                dp->inode = inumber;
                dp->name_len = strlen(name);
                dp->file_type = EXT2_FT_REG_FILE;
                strncpy(dp->name, name, dp->name_len);
                dp->rec_len = BLOCK_SIZE;
                put_block(pip->dev, (pip->INODE).i_block[i + 1], buf);
                break;
            }
        }
    }

    pip->INODE.i_atime = time(0L);
    pip->dirty = 1;

    iput(pip);
}

void
creat_file()
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
        printf("creat : provide a file name\n");
        return;
    }
    ino = getino(&dev, parent);
    pip = iget(dev, ino);

    // verify parent INODE is a DIR and child does not exist in the parent dir
    if ((0040000 == (pip->INODE.i_mode & 0040000)) && (0 == is_exist(pip, child)))
    {
        my_creat(pip, child);
    }
}
