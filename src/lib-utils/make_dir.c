// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * makedir:
 *      Make direcrory
 *      - convenience wrapper handling errors
 *      mode_t (see man 7 inode)
 *
 * A default mode MKDIR_DEF_MODE in sd-boot-utils.h:
 *      MKDIR_MODE_DEF = S_IRWXU | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH
 *
 * makepath:
 *      Makes all the directories in the provided path.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include "sd-boot-msg.h"

/*
 * Make a directory : path
 * All elements in path prior to the last must exist
 */
int makedir(const char *path, mode_t mode) {

    if (!path || path[0] == '\0') {
        return -1;
    }

    if (strcmp(path, "/") == 0) {
        return 0;
    }

    /*
     * Strip trailing slash(es)
     */
    char *path_copy = strdup(path);
    if (!path_copy) {
        perror("memory allocation error");
        return -1;
    }

    size_t len = strlen(path_copy);
    while (len > 1 && path_copy[len - 1] == '/') {
        path_copy[len - 1] = '\0';
        len--;
    }
    
    /*
     * Create dir 
     * if path exists:
     * - ok if a directory
     * - error if a file
     */
    int ret = 0;
    if (mkdir(path_copy, mode) != 0) {
        switch (errno) {
            case EEXIST:        // NOLINT(misc-include-cleaner)
                struct stat stp;
                if (stat(path, &stp) == 0 && S_ISDIR(stp.st_mode)) {
                    ret = 0;
                } else {
                    msg(MSG_ERR, "! mkdir failed - invalid: %s\b", path);
                    ret = -1;
                }
                break;

            default:
                perror("mkdir failed");
                ret = -1;
                break;
        }
    }
    free(path_copy);
    return ret;
}

