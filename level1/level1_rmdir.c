#include <stdbool.h>
#include <time.h>
#include "../filesystem.h"
#include "../util.h"

static bool
is_empty_dir(MINODE *mip)
{
    char replace;
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
    //while (mdp->inode)
    while ((char *) mdp < buf + BLOCK_SIZE)
    {
        replace = mdp->name[mdp->name_len];
        mdp->name[mdp->name_len] = '\0';
        printf("directory name is '%s'\n", mdp->name);
        mdp->name[mdp->name_len] = replace;
        if (0 != strncmp(".", mdp->name, 1) && 0 != strncmp("..", mdp->name, 2))
        {
            return false;
        }
        // Advance to the next DIR struct.
        mdp = (DIR *) (((char *) mdp) + mdp->rec_len);
    }
    return true;
}

int
rm_child(MINODE *pip, char *my_name)
{
    int i;
    char buf[BLOCK_SIZE];
    char replace;
    int shift = 0;
    DIR *prev_dp;
    if (0 == strncmp(".", my_name, 1) || 0 == strncmp("..", my_name, 2))
    {
        // User tried to remove '.' or '..'.
        printf("can't remove . or .. directories\n");
        return -1;
    }
    for (i = 0; i < 12 && (pip->INODE).i_block[i]; i++)
    {
        printf("checking for child to remove in rec block %i\n", (int) (pip->INODE).i_block[i]);
        // Load the next rec block.
        get_block(pip->dev, (pip->INODE).i_block[i], buf);
        shift = 0;
        dp = (DIR *) buf;

        // Make sure we're still within the block.
        while ((char *) dp < (buf + BLOCK_SIZE) && dp->rec_len)
        {
            replace = dp->name[dp->name_len];
            dp->name[dp->name_len] = '\0';
            if (shift)
            {
                if (((char *) dp) + dp->rec_len >= buf + BLOCK_SIZE - 1)
                {
                    printf("found final rec, increasing rec_len by %i bytes\n", shift);
                    dp->rec_len += shift;
                }
                // We already deleted the record and we're shifting the rest
                // down.
                printf("Shifting record for '%s' down by %i bytes.\n", dp->name, shift);
                dp->name[dp->name_len] = replace;
                memmove(((char *) dp) - shift, dp, dp->rec_len);
            }
            else if (0 == strcmp(my_name, dp->name))
            {
                printf("We found a rec structure with name '%s'\n", dp->name);
                dp->name[dp->name_len] = replace;
                if (BLOCK_SIZE == dp->rec_len)
                {
                    printf("It's the only record on this block. Just deleting the block.\n");
                    // This is the only dir record in the block. Just bdealloc
                    // it.
                    bdealloc(pip->dev, (pip->INODE).i_block[i]);
                    (pip->INODE).i_block[i] = 0;
                    (pip->INODE).i_size -= BLOCK_SIZE;
                    return 0;
                }
                else if (((char *) dp) + dp->rec_len >= buf + BLOCK_SIZE)
                {
                    // No need to shift stuff, just extend the previous rec_len
                    // to the end of the block.
                    prev_dp->rec_len += dp->rec_len;
                }
                else
                {
                    // Shift all the directory records down.
                    shift = dp->rec_len;
                    printf("It's not the only record on this block. Shifting other records by %i.\n", shift);
                }
            }
            else
            {
                dp->name[dp->name_len] = replace;
            }
            prev_dp = dp;
            dp = (DIR *) (((char *) dp) + dp->rec_len);
        }
        put_block(pip->dev, (pip->INODE).i_block[i], buf);
    }
    return 0;
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
    printf("rmdir with pathname '%s'\n", pathName);
    // kc notes part 2
    mino = getino(&dev, pathName);
    if (-1 == mino)
    {
        printf("'getino' failed for pathName '%s'\n", pathName);
        return;
    }
    printf("inode number is %i\n", (int) mino);

    // kc notes part 3
    mip = iget(dev, mino);

    // FIXME part 4 and 5 are needed for level 3.

    // kc notes part 6
    // check DIR type && not BUSY && is empty
    //printf("%s i_mode is %x.\n", pathName, MASK_MODE & (mip->INODE).i_mode);
    printf("%s i_mode is %x.\n", pathName, (mip->INODE).i_mode);
    printf("Expected i_mode %x.\n", DIR_MODE);
    if (DIR_MODE != (MASK_MODE & (mip->INODE).i_mode))
    {
        printf("Not a dir.\n");
    }
    else if (RUNNING == running->status)
    {
        printf("Dir is busy.\n");
    }
    else if (!(is_empty_dir(mip)))
    {
        printf("Dir is not empty.\n");
    }
    // It's a dir and it's not busy and it's empty
    else
    {
        // kc nodes part 7
        // Deallocate its block and inode
        for (i = 0; i < 12; i++)
        {
            if (0 != (mip->INODE).i_block[i])
            {
                printf("Deallocating block %i\n", (mip->INODE).i_block[i]);
                bdealloc(mip->dev, (mip->INODE).i_block[i]);
            }
        }
        findino(mip, &mino, &pino);
        idealloc(mip->dev, mip->ino);
        printf("mino is %i, pino is %i\n", (int) mino, (int) pino);

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
    iput(mip);
}
