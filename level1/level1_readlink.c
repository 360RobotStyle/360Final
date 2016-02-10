#include "../filesystem.h"
#include "../util.h"

void
do_readlink()
{

    u32 mino;
    MINODE *mip;
    int dev;

    mino = getino2(&dev, pathName);
    if (-1 == mino)
    {
        printf("readlink : Symlink file '%s' does not exist\n", base_name(pathName));
        return;
    }
    mip = iget(dev, mino);

    if ((0xA000 & (mip->INODE).i_mode) != 0xA000)
    {
        iput(mip);
        printf("readlink : File is not a symlink\n");
        return;
    }
    printf("readlink : '%s' symlink to '%s'\n", pathName, (char *) &((mip->INODE).i_block[16]));
    iput(mip);

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
