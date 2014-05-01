#include "../filesystem.h"
#include "../util.h"

void
do_symlink()
{
    MINODE *pip_from;
    MINODE *mip_from;
    int dev_from;

    MINODE *pip_to;
    MINODE *mip_to;
    int dev_to;

    if (60 > strlen(base_name(pathName)))
    {
        printf("symlink error : Target file name must be less than 60 characters.\n");
        return;
    }
    igetparentandfile(&dev_from, &pip_from, &mip_from, pathName);
    igetparentandfile(&dev_to, &pip_to, &mip_to, pathName);

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
