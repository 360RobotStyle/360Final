#include "filesystem.h"




int main (int argc, char* argv[])
{
    int i;
    command_func cmd;
    char line[128], cname[64];

    init();

    while (1)
    {
        printf("P%d running: ", running->pid);
        printf("input command : ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = '\0';
        if (0 == line[0]) continue;

        sscanf(line, "%s %s %64c", cname, pathname, parameter);

        printf("dirname: %s basename: %s\n", dir_name(), base_name());
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
