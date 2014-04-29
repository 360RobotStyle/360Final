#ifndef __UTIL_H__
#define __UTIL_H__

#include "type.h"

typedef unsigned long u32;

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

char pathName[256];
char pathNameTokenized[256];
char *pathNameTokenPtrs[256];
int tokenCount;

char parameter[256];
char baseName[128];
char dirName[128];

int get_block (int dev, int blk, char* buf);
int put_block (int dev, int blk, char* buf);

/*
 * Tokenize pathname using '/' and '\n' as token delimiters. Store a pointer to
 * each token in the token_ptrs array.
 * Return number of tokens.
 */
int token_path(char *pathname, char **token_ptrs);

/*
 * dirname() and basename() are used to divide a pathname into dirname
 * and basename
 * NOTE: Functions exist in clib.h but it destroys the parameter string
 */
char* dir_name (char* pathname);
char* base_name (char* pathname);

/*
 * Converts a pathname, such as /a/b/c/d OR x/y/z, into its (dev, ino)
 * the returned value is its inumber and dev is its dev number.
 */
u32 getino (int* dev, char* pathname);

/*
 * Searches the data blocks of a DIR inode (inside a MINODE[]) for name
 * Assume DIRECT data blocks only.
 */
u32 search (MINODE* mip, char* name);

MINODE* iget (int dev, unsigned long ino);

/*
 * This function releases MINODE[].
 */
void iput (MINODE* mip);

/*
 * Given parent DIR (MINODE pointer) and my inumber, this function
 * finds the name string of myino in the parent's data block.
 * Similar to search()
 */
int findmyname (MINODE* parent, u32 myino, char* myname);

/*
 * For a DIR MINODE, extract the inumbers of . and ..
 * Read in 0th data block. The inumbers are in the first 2 DIR entries
 */
int findino (MINODE* mip, u32* myino, u32* parent);

/*
 * Increment the free inode count in superblock and group descriptor.
 */
void incFreeInodes(int dev);
/*
 * Increment the free block count in superblock and group descriptor.
 */
void incFreeBlocks(int dev);

/*
 * Deallocate inode.
 */
void idealloc(int dev, u32 ino);

/*
 * Deallocate block.
 */
void bdealloc(int dev, u32 blk);

#endif
