#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
    int inodes_per_block = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int max_inumber = fs->superblock.s_isize * inodes_per_block;
    if (fs == NULL || name == NULL || dirinumber < 1 || dirEnt == NULL || dirinumber > max_inumber) {
        return -1;
    }

    struct inode* inode = malloc(sizeof(struct inode));
    if (inode == NULL) {
        return -1;
    }

    int result = inode_iget(fs, dirinumber, inode);
    if (result == -1) {
        free(inode);
        return -1;
    }

    int bytes_to_read = inode_getsize(inode);
    int blocks_occupied = bytes_to_read / DISKIMG_SECTOR_SIZE;

    int current_block = 0;
    while (current_block <= blocks_occupied) {
        void *buffer = malloc(DISKIMG_SECTOR_SIZE);
        if (buffer == NULL) {
            free(inode);
            return -1;
        }

        int bytes_read = file_getblock(fs, dirinumber, current_block, buffer);
        if (bytes_read == -1) {
            free(buffer);
            free(inode);
            return -1;
        }

        int i = 0;
        while (i < bytes_read) {
            struct direntv6 *dir = (struct direntv6 *)(buffer + i);
            if (strcmp(dir->d_name, name) == 0) {
                memcpy(dirEnt, dir, sizeof(struct direntv6));
                free(buffer);
                free(inode);
                return 0;
            }
            i += sizeof(struct direntv6);
        }
        free(buffer);
        current_block++;
    }

    free(inode);
    return -1;
}

