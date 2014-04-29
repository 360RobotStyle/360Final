#include "../filesystem.h"
#include "../util.h"

void
do_link()
{
    MINODE *dip; // Destination
    MINODE *sip; // Source
    char buf[BLOCK_SIZE];
    u32 dst_ino;
    u32 src_ino;
    int dev;
    int i;

    // We're using pathName as first file path and parameter as second file path.

    dst_ino = getino(&dev, pathName);
    if (-1 == dst_ino)
    {
        printf("Error: Could not find '%s'", pathName);
        return;
    }
    dip = iget(dev, dst_ino);

    if (FILE_MODE != (MASK_MODE & (dip->INODE).i_mode))
    {
        printf("Error: %s is not a file.\n", pathName);
        iput(dip);
        return;
    }
    else if (dir_name(parameter) && -1 == getino(&dev, dir_name(parameter)))
    {
    // Couldn't find the inode for the dir folder we were looking for.
        printf("Error: Couldn't find host folder '%s'\n", dir_name(parameter));
        iput(dip);
        return;
    }
    else if (-1 != getino(&dev, parameter))
    {
        // something already exists where we want to make the link.
        printf("Error: '%s' already exists\n", parameter);
        iput(dip);
        return;
    }
    sip = iget(dev, src_ino); // This is actually the parent of the link src.

    for (i = 0; i < 12, sip->INODE->i_block[i]; i++)
    {
        get_block(sip->dev, sip->INODE->i_block[i], buf);
        dp = (DIR *) buf;
        // TODO FINISH HERE
        // need to add the file name into the directory records.
        //while (((char *) dp) + dp->rec_len
    }




    printf("We're succeeding so far\n");

    // Do we have 
}
//void
//do_ls()
//{
//    int dev;
//    u32 ino;
//    MINODE* mip;
//    char buf[BLOCK_SIZE];
//    char temp[BLOCK_SIZE];
//    char* cp;
//
//    if (0 == pathName[0])
//    {
//        printf("ls : current working directory");
//        ino = running->cwd->ino;
//        dev = running->cwd->dev;
//    }
//    else if (0 == strcmp(pathName, "/"))
//    {
//        printf("ls : root directory");
//        ino = root->ino;
//        dev = root->dev;
//    }
//    else
//    {
//        printf("ls : ");
//        ino = getino(&dev, pathName);
//    }
//
//    if (-1 == ino)
//    {
//        printf("it is not a directory\n");
//        return;
//    }
//    printf("\n");
//
//    mip = iget(dev, ino);
//    ip = &(mip->INODE);
//    get_block(mip->dev, ip->i_block[0], buf);
//    cp = buf;
//    dp = (DIR*)buf;
//    while (cp < buf + BLOCK_SIZE)
//    {
//        strcpy(temp, dp->name);
//        temp[dp->name_len] = 0;
//        file_info(dev, dp->inode);
//        printf("%s\n", temp);
//        cp += dp->rec_len;
//        dp = (DIR*)cp;
//    }
//    iput(mip);
//}
