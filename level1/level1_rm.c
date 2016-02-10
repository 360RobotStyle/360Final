#include "../filesystem.h"
#include "../util.h"

//static int
//rm_child(MINODE *pip, char *my_name)
//{
//    int i;
//    char buf[BLOCK_SIZE];
//    char replace;
//    int shift = 0;
//    if (0 == strncmp(".", my_name, 1) || 0 == strncmp("..", my_name, 2))
//    {
//        // User tried to remove '.' or '..'.
//        printf("can't remove . or .. directories\n");
//        return -1;
//    }
//    for (i = 0; i < 12, (pip->INODE).i_block[i]; i++)
//    {
//        printf("checking for child to remove in dir block %i\n", (int) (pip->INODE).i_block[i]);
//        // Load the next dir block.
//        get_block(pip->dev, (pip->INODE).i_block[i], buf);
//        dp = (DIR *) buf;
//
//        // Make sure we're still within the block.
//        while ((char *) dp < (buf + BLOCK_SIZE) && dp->inode)
//        {
//            replace = dp->name[dp->name_len];
//            dp->name[dp->name_len] = '\0';
//            if (shift)
//            {
//                if (((char *) dp) + dp->rec_len >= buf + BLOCK_SIZE)
//                {
//                    printf("found final rec, increasing rec_len by %i bytes\n", shift);
//                    dp->rec_len += shift;
//                }
//                // We already deleted the record and we're shifting the rest
//                // down.
//                printf("Shifting record for '%s' down by %i bytes.\n", dp->name, shift);
//                dp->name[dp->name_len] = replace;
//                memmove(((char *) dp) - shift, dp, dp->rec_len);
//            }
//            else if (0 == strcmp(my_name, dp->name))
//            {
//                printf("We found a dir structure with name '%s'\n", dp->name);
//                dp->name[dp->name_len] = replace;
//                if (BLOCK_SIZE == dp->rec_len)
//                {
//                    // This is the only dir record in the block. Just bdealloc
//                    // it.
//                    bdealloc(pip->dev, (pip->INODE).i_block[i]);
//                    (pip->INODE).i_block[i] = 0;
//                    // XXX assuming i_size was counting the entire block.
//                    (pip->INODE).i_size -= BLOCK_SIZE;
//                    return 0;
//                }
//                else
//                {
//                    // Shift all the directory records down.
//                    shift = dp->rec_len;
//                }
//            }
//            else
//            {
//                dp->name[dp->name_len] = replace;
//            }
//            dp = (DIR *) (((char *) dp) + dp->rec_len);
//            // TODO Finish. We need to find the final record and increase it's
//            // length by shift amount.
//        }
//        put_block(pip->dev, (pip->INODE).i_block[i], buf);
//    }
//    return 0;
//}

void
do_rm()
{
    u32 mino;
    u32 pino;
    MINODE* mip;
    MINODE* pip;
    int i;
    int dev;
    printf("rm with pathname '%s'\n", pathName);
    // kc notes part 2
    if (dir_name(pathName))
    {
        pino = getino(&dev, dir_name(pathName));
    }
    else
    {
        pino = getino(&dev, ".");
    }

    if (-1 == pino)
    {
        printf("'getino' failed for pathName '%s'\n", dir_name(pathName));
        return;
    }
    pip = iget(dev, pino);
    printf("parent inode number is %i\n", (int) pip->ino);


    mino = getfileino(pip, base_name(pathName));
    if (-1 == mino)
    {
        iput(pip);
        printf("'getino' failed for file '%s'\n", base_name(pathName));
        return;
    }
    printf("mino is %i\n", (int) mino);

    // kc notes part 3
    mip = iget(dev, mino);

    if (1 < (mip->INODE).i_links_count)
    {
        printf("File has active links. Must unlink first.\n");
        iput(pip);
        iput(mip);
        return;
    }

    // FIXME part 4 and 5 are needed for level 3.

    // kc notes part 6
    // check DIR type && not BUSY && is empty
    //printf("%s i_mode is %x.\n", pathName, MASK_MODE & (mip->INODE).i_mode);
    printf("%s i_mode is %x.\n", pathName, (mip->INODE).i_mode);
    printf("Expected i_mode %x.\n", FILE_MODE);
    if (FILE_MODE != (MASK_MODE & (mip->INODE).i_mode))
    {
        printf("Not a file.\n");
    }
    else if (RUNNING == running->status)
    {
        printf("File is busy.\n");
    }
    // It's a file and it's not busy
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
                printf("Deallocated block %i\n", (mip->INODE).i_block[i]);
            }
        }
        printf("finding inode\n");
        //findino(mip, &mino, &pino);
        pino = getino(&dev, dir_name(pathName));
        if (-1 == pino)
        {
            pino = getino(&dev, "..");
        }
        printf("Deallocating inode %i\n", mip->ino);
        idealloc(mip->dev, mip->ino);
        printf("Deallocated inode %i\n", mip->ino);
        //printf("mino is %i, pino is %i\n", (int) mino, (int) pino);

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
