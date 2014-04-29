#include <stdbool.h>
#include <time.h>
#include "../filesystem.h"
#include "../util.h"

static bool
is_empty_dir(MINODE *mip)
{
    DIR *mdp; // my dir pointer
    char buf[BLOCK_SIZE];
    // We don't need to worry about anything past block 0. If something besides
    // . and .. exists, return false immediately.
    get_block(mip->dev, (mip->INODE).i_block[0], buf);
    mdp = (DIR *) buf;

    // Check for . and .. directories. Fail if we see anything else.
    //
    // XXX is there any possibility that mdp->inode could be junk? I'm assuming
    // the empty space in the block is zeroed out.
    while (mdp->inode)
    {
        if (0 != strncmp(".", mdp->name, 1) && 0 != strncmp("..", mdp->name, 2))
        {
            return false;
        }
        // Advance to the next DIR struct.
        mdp = (DIR *) (((char *) mdp) + mdp->rec_len);
    }
    return true;
}

static int
rm_child(MINODE *pip, char *my_name)
{
    int i;
    char buf[BLOCK_SIZE];
    char replace;
    char *cp;
    DIR* prev_dp;
    int shift = 0;
    if (0 == strncmp(".", my_name, 1) || 0 == strncmp("..", my_name, 2))
    {
        printf("can't remove . or .. directories\n");
        return -1;
    }
    for (i = 0; i < 12, (pip->INODE).i_block[i]; i++)
    {
        // Load dir structures.
        get_block(pip->dev, (pip->INODE).i_block[i], buf);
        dp = (DIR *) buf;
        cp = buf;

        // Make sure we're still within the block.
        while ((char *) dp < (buf + BLOCK_SIZE) && dp->inode)
        {
            replace = dp->name[dp->name_len];
            dp->name[dp->name_len] = '\0';
            if (shift)
            {
                // We already deleted the record and we're shifting the rest
                // down.
                dp->name[dp->name_len] = replace;
                memmove(((char *) dp) - shift, dp, dp->rec_len);
            }
            else if (0 == strcmp(my_name, dp->name))
            {
                if (BLOCK_SIZE == dp->rec_len)
                {
                    // This is the only dir record in the block. Just bdealloc
                    // it.
                    bdealloc(pip->dev, (pip->INODE).i_block[i]);
                    (pip->INODE).i_block[i] = 0;
                    // XXX assuming i_size was counting the entire block.
                    (pip->INODE).i_size -= BLOCK_SIZE;
                    return 0;
                    // FIXME we need to fix some other stuff in the inode.
                }
                else
                {
                    // Shift all the directory records down.
                    shift = dp->rec_len;
                }
            }
            dp->name[dp->name_len] = replace;
            prev_dp = (DIR *) (((char *) dp) - dp->rec_len);
            dp = (DIR *) (((char *) dp) + dp->rec_len);
            // TODO Finish. We need to find the final record and increase it's
            // length by shift amount.
        }
        if (shift)
        {
            // Increase the final record's length to account for the deleted
            // record.
            prev_dp->rec_len += shift;
            return 0;
        }
    }
    return -1;
}

void
do_rmdir()
{
    u32 mino;
    u32 pino;
    MINODE* mip;
    MINODE* pip;
    int i;
    int dev;
    printf("getting to rmdir with pathname '%s'\n", pathName);
    // kc notes part 2
    mino = getino(&dev, pathName);

    // kc notes part 3
    mip = iget(dev, mino);

    // FIXME part 4 and 5 are needed for level 3.

    // kc notes part 6
    // check DIR type && not BUSY && is empty
    printf("%s i_mode is %o.\n", pathName, (mip->INODE).i_mode);
    printf("Expected i_mode %o.\n", DIR_MODE);
    if (DIR_MODE != (mip->INODE).i_mode)
    {
        printf("Not a dir.\n");
        iput(mip);
    }
    else if (RUNNING == running->status)
    {
        printf("Dir is busy.\n");
        iput(mip);
    }
    else if (!(is_empty_dir(mip)))
    {
        printf("Dir is not empty.\n");
        iput(mip);
    }
    // It's a dir and it's not busy and it's impty
    else
    {
        // kc nodes part 7
        // Deallocate its block and inode
        for (i = 0; i < 12; i++)
        {
            if (0 != (mip->INODE).i_block[i])
            {
                bdealloc(mip->dev, (mip->INODE).i_block[i]);
            }
        }
        idealloc(mip->dev, mip->ino);
        iput(mip);
        findino(mip, &mino, &pino);

        // kc notes part 8
        pip = iget(mip->dev, pino);

        // kc notes part 9
        rm_child(pip, base_name(pathName));

        // kc notes part 10
        //(pip->INODE).i_links_count--; // XXX taken care of in idalloc?
        (pip->INODE).i_atime = time(NULL);
        (pip->INODE).i_mtime = (pip->INODE).i_atime;
        pip->dirty = 1;
        iput(pip);
    }
}
