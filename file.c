#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    int inodes_per_block = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int max_inumber = fs->superblock.s_isize * inodes_per_block;
    if (fs == NULL || inumber < 1 || blockNum < 0 || buf == NULL || inumber > max_inumber) {
        return -1;
    }

    struct inode* inode = malloc(sizeof(struct inode));
    if (inode == NULL) {
        return -1;
    }

    int result = inode_iget(fs, inumber, inode);
    if (result == -1) {
        free(inode);
        return -1;
    }
    
    int bytes_to_read = inode_getsize(inode);
    int blocks_occupied = bytes_to_read / DISKIMG_SECTOR_SIZE;
    if (blockNum > blocks_occupied) {
        free(inode);
        return -1;
    }

    int block_in_disk = inode_indexlookup(fs, inode, blockNum);
    if (block_in_disk == -1) {
        free(inode);
        return -1;
    }

    int bytes_to_read_in_this_block = (blockNum == blocks_occupied) ? bytes_to_read % DISKIMG_SECTOR_SIZE : DISKIMG_SECTOR_SIZE;
    void *buffer = malloc(DISKIMG_SECTOR_SIZE);
    if (buffer == NULL) {
        free(inode);
        return -1;
    }

    int bytes_read = diskimg_readsector(fs->dfd, block_in_disk, buffer);
    if (bytes_read == -1) {
        free(buffer);
        free(inode);
        return -1;
    }

    memcpy(buf, buffer, bytes_to_read_in_this_block);
    free(buffer);
    free(inode);

    return bytes_to_read_in_this_block;
}

