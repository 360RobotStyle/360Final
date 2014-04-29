#include "filesystem.h"




int main (int argc, char* argv[])
{
    int i;
    command_func cmd;
    char line[128], cname[64];

    init();

    while (1)
    {
        bzero(cname, 64);
        bzero(line, 64);
        bzero(pathName, 256);
        bzero(pathNameTokenized, 256);
        bzero(pathNameTokenPtrs, 256);
        bzero(baseName, 128);
        bzero(dirName, 128);
        bzero(parameter, 128);

        printf("\033[2mP%d running: ", running->pid);
        printf("input command : \033[0m");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = '\0';
        if (0 == line[0]) continue;

        sscanf(line, "%s %s %64c", cname, pathName, parameter);

        //printf("dirname: %s basename: %s\n", dir_name(pathName), base_name(pathName));
        cmd = findCmd(cname);
        if ((int) cmd)
        {
            cmd();
        }
        else
        {
            printf("invalid command\n");
        }
    }

    return 0;
} /* end main */
