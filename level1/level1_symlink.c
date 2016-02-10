#include "../filesystem.h"
#include "../util.h"

static void
my_symlink_creat(MINODE* pip, char* name)
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
    mip->INODE.i_mode = 0x8000;
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

    //iput(pip);
}

void
do_symlink()
{
    MINODE *pip_from;
    MINODE *mip_from;
    u32 mino_from;
    int dev_from;

    MINODE *pip_to;
    MINODE *mip_to;
    MINODE *mip;
    u32 mino_to;
    int dev_to;

    int dev;
    u32 ino;
    u32 mino;
    char* parent;
    char* child;
    MINODE* pip;

    if (60 <= strlen(pathName))
    {
        printf("symlink error : Target file name must be less than 60 characters.\n");
        return;
    }

    mino_from = getino2(&dev_from, parameter);
    if (-1 != mino_from)
    {
        printf("symlink : a file with the name '%s' already exists\n", base_name(parameter));
        return;
    }

    mino_to = getino2(&dev_to, pathName);
    if (-1 == mino_to)
    {
        printf("symlink : target file '%s' does not exist\n", base_name(pathName));
        return;
    }

    //mip_to = iget(dev_to, mino_to);
    //mip_from = ialloc(mip_to->dev);

    if ('/' == parameter[0])
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    parent = dir_name(parameter);
    child = base_name(parameter);

    if (0 == strcmp(child, "."))
    {
        printf("symlink : cannot create file '.'\n");
        return;
    }
    ino = getino(&dev, parent);
    pip = iget(dev, ino);

    // verify parent INODE is a DIR and child does not exist in the parent dir
    if ((0040000 == (pip->INODE.i_mode & 0040000)) && (0 == is_exist(pip, child)))
    {
        my_symlink_creat(pip, child);
    }
    mino = getfileino(pip, child);
    mip = iget(dev, mino);
    mip->INODE.i_mode = 0xA000;
    iput(pip);

    strcpy((char *) &((mip->INODE).i_block[16]), pathName);
    printf("created link to '%s'\n", (char *) &((mip->INODE).i_block[16]));
    printf("in inode %i\n", mip->ino);
    iput(mip);

    //mip_to = getino(&dev_to, mino_to);

    //igetparentandfile(&dev_to, &pip_to, &mip_to, pathName);
    //igetparentandfile(&dev_from, &pip_from, &mip_from, parameter);

    //if (dev_from != dev_to)
    //{
    //    printf("symlink error : Cannot link to a file on another device.\n");
    //    cleanup_symlink(pip_from, mip_from, pip_to, mip_to);
    //    return;
    //}

    //if (!pip_from)
    //{
    //    printf("symlink error : Directory '%s' does not exist.\n", dir_name(parameter));
    //    cleanup_symlink(pip_from, mip_from, pip_to, mip_to);
    //    return;
    //}
    //else if (mip_from)
    //{
    //    printf("symlink error : File '%s' already exists.\n", base_name(parameter));
    //    cleanup_symlink(pip_from, mip_from, pip_to, mip_to);
    //    return;
    //}
    //else if (!mip_to)
    //{
    //    printf("symlink error : Target file '%s' does not exists.\n", base_name(pathName));
    //    cleanup_symlink(pip_from, mip_from, pip_to, mip_to);
    //    return;
    //}
    //else if (
    //cleanup_symlink(pip_from, mip_from, pip_to, mip_to);



// 3. symlink oldNAME  newNAME    e.g. symlink /a/b/c /x/y/z
// ASSUME: oldNAME has <= 60 chars, inlcuding the NULL byte.
// (INODE has 24 UNUSED bytes after i_block[]. So may use up to 84 bytes for oldNAME) 

    


// (1). verify oldNAME exists (either a DIR or a FILE)
// (2). creat a FILE /x/y/z
// (3). change /x/y/z's type to S_IFLNK (0120000)=(1010.....)=0xA...
// (4). write the string oldNAME into the i_block[ ], which has room for 60 chars.
// (5). write the INODE of /x/y/z back to disk.
// 
// 4. readlink pathname: return the contents of a symLink file
// (1). get INODE of pathname into a minode[ ].
// (2). check INODE is a symbolic Link file.
// (3). return its string contents in INODE.i_block[ ].
}
