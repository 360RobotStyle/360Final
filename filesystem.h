#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include <stdlib.h> // exit(1)
#include <time.h>

#include "util.h"

typedef void (*command_func)();
typedef struct command
{
    char const *name;
    command_func func;
} command;


void init ();
void mount_root ();
command_func findCmd(char* cname);

#endif
