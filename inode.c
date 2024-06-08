#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#include "diskimg.h"

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    int inodes_per_block = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int max_inumber = fs->superblock.s_isize * inodes_per_block;

    if (fs == NULL || inumber < 1 || inp == NULL || inumber > max_inumber) {
        return -1;
    }

    int block_num = ((inumber - 1) / inodes_per_block) + INODE_START_SECTOR;

    void *buffer = malloc(DISKIMG_SECTOR_SIZE);
    if (buffer == NULL) {
        return -1;
    }

    int bytes_read = diskimg_readsector(fs->dfd, block_num, buffer);
    if (bytes_read == -1) {
        free(buffer);
        return -1;
    }

    int byte_num = ((inumber - 1) % inodes_per_block) * sizeof(struct inode);

    memcpy(inp, buffer + byte_num, sizeof(struct inode));
    free(buffer);

    return 0;
}

int one_level_indirection(struct unixfilesystem *fs, struct inode *inp, int blockNum, int block_num_pointer) {
    void *buffer = malloc(DISKIMG_SECTOR_SIZE);

    if (buffer == NULL) {
        return -1;
    }

    int bytes_read = diskimg_readsector(fs->dfd, inp->i_addr[block_num_pointer], buffer);
    if (bytes_read == -1) {
        free(buffer);
        return -1;
    }
    
    int byte_num = (blockNum % (DISKIMG_SECTOR_SIZE / sizeof(uint16_t))) * sizeof(uint16_t);

    int block_num_disk = *((uint16_t *)(buffer + byte_num));

    free(buffer);

    return block_num_disk;

}

int two_level_indirection(struct unixfilesystem *fs, struct inode *inp, int blockNum, int block_nums_per_block) {
    int block_num_pointer2 = (blockNum - (7 * block_nums_per_block)) / block_nums_per_block;

    void *buffer = malloc(DISKIMG_SECTOR_SIZE);

    if (buffer == NULL) {
        return -1;
    }

    int bytes_read = diskimg_readsector(fs->dfd, inp->i_addr[7], buffer);

    if (bytes_read == -1) {
        free(buffer);
        return -1;
    }

    int block_num_pointer2_disk = *((uint16_t *)(buffer + block_num_pointer2 * 2));

    bytes_read = diskimg_readsector(fs->dfd, block_num_pointer2_disk, buffer);

    if (bytes_read == -1) {
        free(buffer);
        return -1;
    }

    int byte_num = (blockNum % block_nums_per_block) * sizeof(uint16_t);

    int block_num_disk = *((uint16_t *)(buffer + byte_num));

    free(buffer);

    return block_num_disk;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    int block_nums_per_block = DISKIMG_SECTOR_SIZE / sizeof(uint16_t);
    if (fs == NULL || inp == NULL || blockNum < 0) {
        return -1;
    }

    if (blockNum * DISKIMG_SECTOR_SIZE > inode_getsize(inp)) {
        return -1;
    }

    int file_size = inode_getsize(inp);
    if ((inp->i_mode & ILARG) == 0) { // small file
        return inp->i_addr[blockNum];
    }
    int block_num_pointer = blockNum / (DISKIMG_SECTOR_SIZE / sizeof(uint16_t));

    if (block_num_pointer < 7) { // only one level of indirection
    return one_level_indirection(fs, inp, blockNum, block_num_pointer);
    }
    else { // two levels of indirection
    return two_level_indirection(fs, inp, blockNum, block_nums_per_block);
    }
}


int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
