#include "filesystem.h"
#include "level1/level1.h"
#include "level2/level2.h"
#include "level3/level3.h"

// Matches command line commands to associated function pointer.
static command command_table[] =
{
    // LEVEL 1
    {"menu",    menu},
    {"mkdir",   make_dir}, // Gabe
    {"cd",      do_cd}, // Cameron
    {"pwd",     do_pwd}, // Cameron
    {"ls",      do_ls}, // Gabe
    {"rmdir",   do_rmdir}, // Cameron
    {"creat",   creat_file}, // Gabe
    {"link",    do_link}, // Cameron
    {"unlink",  do_unlink}, // Cameron
    //{"symlink", symlink}, // Cameron
    {"rm",      do_rm}, // Cameron
    {"chmod",   chmod_file}, // Gabe
    {"chown",   chown_file}, // Gabe
    {"chgrp",   chgrp_file}, // Gabe
    {"stat",    stat_file}, // Gabe
    {"touch",   touch_file}, // Gabe

    // LEVEL 2
    {"open",    do_open}, // Cameron
    //{"close",   do_close}, // Cameron
    {"pfd",     pfd},
    //{"lseek",   do_lseek}, // Cameron
    //{"rewind",  access_file},
    {"read",    read_file},
    {"write",   write_file}, // Gabe
    //{"cat",     cat_file},
    //{"cp",      cp_file}, // Gabe
    //{"mv",      mv_file}, // Gabe

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

    for (i = 0; i < NFD; i++) proc[0].fd[i] = 0; // set to null
    for (i = 0; i < NOFT; i++) oft[i].refCount = 0;

    proc[1].uid = 1;
    proc[1].cwd = 0;

    running = &proc[0];
    readyQueue = &proc[1];

    for (i = 0; i < NMINODES; i++) minode[i].refCount = 0;

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
    printf("\033[2menter rootdev name (RETURN for /dev/fd0): \033[0m");
    fgets(device, 128, stdin);
    device[strlen(device) - 1] = 0;

    // open device for RW
    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        err_printf("error: can't open root device\n");
        exit(-1);
    }

    // check InodesBeginBlock
    get_block(fd, GDBLOCK, buf);
    gp = (GD*)buf;
    if (INODEBLOCK != gp->bg_inode_table)
    {
        err_printf("error: inode begin block\n");
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
        err_printf("error: not ext2 filesystem\n");
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






