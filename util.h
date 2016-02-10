#ifndef __UTIL_H__
#define __UTIL_H__

#include "type.h"

#define DEBUG (1)

#ifndef DEBUG
#define DEBUG (0)
#endif

// Debug printing macro. Enable it by defining DEBUG as 1.
#define DEBUG_PRINT(...) \
    do \
    { \
        if (DEBUG) \
        { \
            fprintf(stderr, "%s(), line %i: ", __func__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
        } \
    } while (0)

typedef unsigned long u32;

// type.h
extern GD    *gp;
extern SUPER *sp;
extern INODE *ip;
extern DIR   *dp;


INODE myinode;
PROC proc[NPROC];
PROC* running;
PROC* readyQueue;
MINODE minode[NMINODES];
MINODE* root;
OFT oft[NOFT];
MOUNT mount[NMOUNT];

char line[256];
char pathName[128];
char pathNameTokenized[128];
char *pathNameTokenPtrs[128];
int tokenCount;

char parameter[64];
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

// alternate version that doesn't fail for file types.
u32 getino2 (int* dev, char* pathname);

/*
 * Use getino to find the parent inode number, get the parent minode, and then
 * get the file.
 */
u32 getfileino(MINODE *pip, char* name);

/*
 * Searches the data blocks of a DIR inode (inside a MINODE[]) for name
 * Assume DIRECT data blocks only.
 */
u32 search (MINODE* mip, char* name);

// alternate version that doesn't fail for file types.
u32 search2 (MINODE* mip, char* name);

int is_exist (MINODE* mip, char* name);

MINODE* iget (int dev, unsigned long ino);
void igetparentandfile(int *dev, MINODE **pip, MINODE **mip, char *name);

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

/*
 * Decrement the free inode count in superblock and group descriptor
 */
void decFreeInodes(int dev);

/*
 * Allocate inode
 */
int ialloc (int dev);

MOUNT *oalloc (int dev);
void odealloc (int dev);

/*
 * Decrement the free blocks count in superblock and group descriptor
 */
void decFreeBlocks(int dev);

/*
 * Allocate block
 */
int balloc (int dev);

int TST_bit (char buf[], int BIT);

int SET_bit (char buf[], int BIT);

int CLR_bit (char buf[], int BIT);

/*
 * Remove child of folder pointed to by pip with name my_name. Make sure pip is
 * a DIR_MODE file before calling this.
 */
int rm_child(MINODE *pip, char *my_name);

/*
 * Insert a DIR into the directory blocks of pip->INODE.
 */
int put_rec(MINODE *pip, char *name, u32 ino);
int del_rec(MINODE *pip, char *name);

/*
 * Allocate entry from open file table
 */
OFT* falloc();

void err_printf(char* msg);

#endif
