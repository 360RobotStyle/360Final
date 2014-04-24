#ifndef __UTIL_H__
#define __UTIL_H__

#include "type.h"

// type.h
extern GD    *gp;
extern SUPER *sp;
extern INODE *ip;
extern DIR   *dp;


INODE myinode;
PROC proc[2];
PROC* running;
PROC* readyQueue;
MINODE minode[NMINODES];
MINODE* root;


int get_block(int dev, int blk, char* buf);
int put_block(int dev, int blk, char* buf);

/*
 * This function breaks up a pathname, such as /a/b/c/d, into
 * components a  b  c  d and determines the number of components n.
 * The components will be used to search for the inode of pathname.
 */
int token_path(char *pathname);

/*
 * dirname() and basename() are used to divide a pathname into dirname
 * and basename
 * NOTE: Functions exist in clib.h but it destroys the parameter string
 */
//int dirname(char* string);
//int basename(char* string);


MINODE* iget (int dev, unsigned long ino);


#endif
