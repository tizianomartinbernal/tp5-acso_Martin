#ifndef _INODE_H
#define _INODE_H

#include "unixfilesystem.h"
#include "diskimg.h"

/**
 * Fetches the specified inode from the filesystem. 
 * Returns 0 on success, -1 on error.  
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp); 

/**
 * Given an index of a file block, retrieves the file's actual block number
 * of from the given inode.
 *
 * Returns the disk block number on success, -1 on error.  
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum);

/**
 * Computes the size in bytes of the file identified by the given inode
 */
int inode_getsize(struct inode *inp);

/*
* Given an index of a file block, retrieves the file's actual block number
*/
int two_level_indirection(struct unixfilesystem *fs, struct inode *inp, int blockNum, int block_nums_per_block);

/*
* Given an index of a file block, retrieves the file's actual block number
*/
int one_level_indirection(struct unixfilesystem *fs, struct inode *inp, int blockNum, int block_nums_per_block);


#endif // _INODE_
