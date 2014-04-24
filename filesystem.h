#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include <stdlib.h> // exit(1)
#include <time.h>

#include "util.h"

typedef unsigned long u32;

void init ();
void mount_root ();
int findCmd(char* cname);

/* LEVEL 1 */
void menu ();
void make_dir();
void change_dir();

void quit();

#endif
