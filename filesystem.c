#include "filesystem.h"
#include "level1/level1.h"
#include "level2/level2.h"
#include "level3/level3.h"

// Matches command line commands to associated function pointer.
static command command_table[] =
{
    // LEVEL 1
    {"menu",    menu},
    {"mkdir",   make_dir},
    {"cd",      do_cd},
    {"pwd",     do_pwd},
    {"ls",      do_ls},
    {"rmdir",   do_rmdir}, // cameron
    //{"creat",   creat_file},
    //{"link",    link}, // cameron
    //{"unlink",  unlink}, // cameron
    //{"symlink", symlink}, // cameron
    {"rm",      do_rm}, // cameron
    //{"chmod",   chmod_file},
    //{"chown",   chown_file},
    {"stat",    stat_file},
    //{"touch",   touch_file},

    // LEVEL 2
    //{"open",    open_file},
    //{"close",   close_file},
    //{"pfd",     pfd},
    //{"lseek",   lseek_file},
    //{"rewind",  access_file},
    //{"read",    read_file},
    //{"write",   write_file},
    //{"cat",     cat_file},
    //{"cp",      cp_file},
    //{"mv",      mv_file},

    // LEVEL 3
    //{"mount",   mount},
    //{"umount",  umount},
    //{"cs",      cs},
    //{"fork",    do_fork},
    //{"ps",      do_ps},
    //{"kill",    do_kill},
    //{"sync",    sync},
    {"quit",    quit},
    {NULL,      NULL}
};

// Initialize data structures of LEVEL-1
void init ()
{
    int i;

    proc[0].uid = SUPER_USER;
    proc[0].cwd = 0;

    proc[1].uid = 1;
    proc[1].cwd = 0;

    running = &proc[0];
    readyQueue = &proc[1];

    for (i = 0; i < NMINODES; i++) { minode[i].refCount = 0; }

    root = 0;

    mount_root();
}

// mount root file system, establish / and CWDs
void mount_root ()
{
    char buf[BLOCK_SIZE];
    char device[128];
    int fd;

    // Prompt user
    printf("mounting root\n");
    printf("enter rootdev name (RETURN for /dev/fd0): ");
    fgets(device, 128, stdin);
    device[strlen(device) - 1] = 0;

    // open device for RW
    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "error: can't open root device\n");
        exit(-1);
    }

    // check InodesBeginBlock
    get_block(fd, GDBLOCK, buf);
    gp = (GD*)buf;
    if (INODEBLOCK != gp->bg_inode_table)
    {
        fprintf(stderr, "error: inode begin block not equal to %d\n", INODEBLOCK);
        exit(-1);
    }

    printf("iblock=%d\n", INODEBLOCK);
    printf("imap=%d\n", IBITMAP);
    printf("bmap=%d\n", BBITMAP);

    printf("mount : %s mounted on /", device);

    // read SUPER block to verify it's an EXT2 FS
    get_block(fd, SUPERBLOCK, buf);
    sp = (SUPER*)buf;
    if (SUPER_MAGIC != sp->s_magic)
    {
        fprintf(stderr, "error: not ext2 filesystem\n");
        exit(-1);
    }
    printf("nblocks=%d  bfree=%d   ninodes=%d  ifree=%d\n",
                sp->s_blocks_count, sp->s_free_blocks_count,
                sp->s_inodes_count, sp->s_free_inodes_count);

    // Get root inode
    root = iget(fd, ROOT_INODE);
    printf("mounted root\n");

    // Let cwd of both P0 and P1 point at the root minode (refCOunt = 3)
    printf("creating P0, P1\n");
    proc[0].cwd = root;
    proc[1].cwd = root;
}

// Find command pointer associated with given terminal input command.
command_func findCmd(char* cname)
{
    command *curr_command;
    curr_command = command_table;
    // Find function pointer in command table associated with given cname or
    // NULL if cname does not match any commands.
    while (curr_command->name && 0 != strcmp(curr_command->name, cname))
    {
        curr_command++;
    }
    // Return the function pointer or NULL.
    return curr_command->func;
}

//INODE* _iget(int fd, int ino)
//{
    //int blk, offset;
    //char buf[BLOCK_SIZE];

    //blk = (ino - 1)/8 + InodesBeginBlock;
    //offset = (ino - 1) % 8;
    ////printf("ino %d  blk %d  offset %d\n", ino, blk, offset);

    //get_block(fd, blk, buf);
    //ip = (INODE*)buf + offset;
    //myinode = *ip;

    //return &myinode;
//}




//void iput (MINODE* mip)
//{
    //int blk, offset;

    //// Dispose of an minode pointed by mip:
    //mip->refCount--;

    //if (mip->refCount > 0) { return; }
    //if (!mip->dirty) { return; }


    //if (mip->refCount == 0 && mip->dirty)
    //{
        //// Write INODE back to the disk by its (dev, ino)
        //blk = (mip->ino - 1)/8 + InodesBeginBlock;
        //offset = (mip->ino - 1) % 8;
        //lseek(mip->dev, (long)(blk*BLOCK_SIZE), 0);
        //write(mip->dev, mip->inode, BLOCK_SIZE);
        //mip->dirty = 0;
    //}
//}


//int change_dir (char* pathname)
//{
    //int ino, dev;
    //MINODE* mip;

    //if (pathname[0] == 0)
    //{
        //iput(running->cwd); // dispose of cwd
        //running->cwd = root;
        //root->refCount++;
        //return GOOD;
    //}

    //ino = getino(&dev, pathname); // DOUBLE CHECK THIS CODE

    //if (ino == 0)
    //{
        //printf("Directory not found\n");
        //return BAD;
    //}

    //mip = iget(dev, ino);

    //// Check it's a directory
    //if ((mip->inode.i_mode & 0040000) != 0040000)
    //{
        //iput(mip);
        //printf("Not a directory\n");
        //return BAD;
    //}

    //// Dispose of original running->cwd
    //running->cwd = mip;

    //return GOOD;
//}







