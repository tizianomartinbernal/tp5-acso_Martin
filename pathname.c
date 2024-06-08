
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || strlen(pathname) == 0 || fs == NULL || pathname[0] != '/') {
        return -1;
    }

    char path[strlen(pathname)];
    strcpy(path, pathname + 1);

    int inumber = 1;

    struct direntv6* dir_ent = malloc(sizeof(struct direntv6));
    if (dir_ent == NULL) {
        return -1;
    }

    char *token = strtok(path, "/");

    while (token != NULL) {
        int result = directory_findname(fs, token, inumber, dir_ent);
        if (result == -1) {
            free(dir_ent);
            return -1;
        }
        inumber = dir_ent->d_inumber;
        token = strtok(NULL, "/");
    }
    
    free(dir_ent);
    return inumber;
}