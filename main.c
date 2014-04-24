#include "filesystem.h"



int main (int argc, char* argv[])
{
    int i, cmd;
    char line[128], cname[64];

    init();

    while (1)
    {
        printf("P%d running: ", running->pid);
        printf("input command : ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = 0;
        if (0 == line[0]) continue;

        sscanf(line, "%s %s %64c", cname, pathname, parameter);

        printf("dirname: %s basename: %s\n", dir_name(), base_name());
        cmd = findCmd(cname);

        // Change to Table of Function Pointers later
        switch (cmd)
        {
            // ----------- LEVEL 1 -------------
            case 0 : menu();              break;
            case 1 : /*make_dir();*/          break;
            case 2 : change_dir();        break;
            case 3 : /*pwd(cwd);*/            break;
            case 4 : /*list_dir();*/          break;
            case 5 : /*rmdir();*/             break;
            case 6 : /*creat_file();*/        break;
            case 7 : /*link();*/              break;
            case 8 : /*unlink();*/            break;
            case 9 : /*symlink();*/           break;
            case 10: /*rm_file();*/           break;
            case 11: /*chmod_file();*/        break;
            case 12: /*chown_file();*/        break;
            case 13: /*stat_file();*/         break;
            case 14: /*touch_file();*/        break;

            // ----------- LEVEL 2 -------------
            case 20: /*open_file();*/         break;
            case 21: /*close_file();*/        break;
            case 22: /*pfd();*/               break;
            case 23: /*lseek_file();*/        break;
            case 24: /*access_file();*/       break; // rewind()
            case 25: /*read_file();*/         break;
            case 26: /*write_file();*/        break;
            case 27: /*cat_file();*/          break;
            case 28: /*cp_file();*/           break;
            case 29: /*mv_file();*/           break;

            // ----------- LEVEL 3 -------------
            case 30: /*mount();*/             break;
            case 31: /*umount();*/            break;
            case 32: /*cs;()*/                break;
            case 33: /*do_fork();*/           break;
            case 34: /*do_ps();*/             break;
            case 35: /*do_kill();*/           break;

            case 40: /*sync();*/              break;
            case 41: quit();               break;
            default: printf("invalid command\n");
                     break;
        }
    }

    return 0;
} /* end main */
