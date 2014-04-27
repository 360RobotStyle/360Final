#include <stdbool.h>
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
        mdp = ((char *) mdp) + mdp->rec_len;
    }
    return true;
}

void
rmdir()
{
    u32 mino;
    u32 pino;
    MINODE* mip;
    MINODE* pip;
    int i;
    // kc notes part 2
    mino = getino(running->cwd->dev, pathName);

    // kc notes part 3
    mip = iget(running->cwd->dev, mino);

    // FIXME part 4 and 5 are needed for level 3.

    // kc notes part 6
    // check DIR type && not BUSY && is empty
    if (DIR_MODE == (mip->INODE).i_mode && BUSY != running->status)
    {
        if (is_empty_dir(mip))
        {
            // kc nodes part 7
            // Deallocate its block and inode
            for (i = 0; i < 12; i++)
            {
                if (0 != (mip->INODE).i_block[i])
                {
                    bdalloc(mip->dev, (mip->INODE).i_block[i]);
                }
            }
            idalloc(mip->dev, mip->ino);
            iput(mip);
            findino(mip, &mino, &pino);
            pip = iget(dev, pino, pathName);
            // FIXME finish here.
        }
        // It's a dir and it's not busy. Make sure it's empty too.
    }

    iput(mip);

}
