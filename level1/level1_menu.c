#include "../filesystem.h"
#include "../util.h"

void
menu()
{
    printf("******************** Menu *******************\n"
           "mkdir     creat     mount     umount    rmdir\n"
           "cd        ls        pwd       stat      rm\n"
           "link      unlink    symlink   chmod     chown   touch\n"
           "open      pfd       lseek     rewind    close\n"
           "read      write     cat       cp        mv\n"
           "cs        fork      ps        kill      quit\n"
           "=============   Usage Examples ==============\n"
           "mkdir  filename\n"
           "mount  filesys   /mnt\n"
           "chmod  filename  0644\n"
           "chown  filename  uid\n"
           "open   filename  mode (0|1|2|3 for R|W|RW|AP)\n"
           "write  fd  text_string\n"
           "read   fd  nbytes\n"
           "pfd    (display opened file descriptors)\n"
           "cs     (switch process)\n"
           "fork   (fork child process)\n"
           "ps     (show process queue as Pi[uid]==>}\n"
           "kill   pid   (kill a process)\n"
           "*********************************************\n");    
}
