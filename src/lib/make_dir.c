// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * makedir:
 *      Make direcrory
 *      - convenience wrapper handling errors
 *      mode_t (see man 7 inode)
 *      If mode is 0 the default mode is 0755
 *      same as : S_IRWXU | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH
 * makepath:
 *      Makes all the directories in the provided path.
 */
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include "sd-boot.h"

/*
 * Make a directory : path
 * All elements in path prior to the last must exist
 */
int makedir(const char *path, mode_t mode) {
    int ret = 0;

    if (mode == 0) {
        mode = S_IRWXU | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH;
    }

    if (mkdir(path, mode) != 0) {
        switch (errno) {
            case EEXIST:
                break;

            default:
                ret = -1;
                perror(nullptr);
                break;
        }
    }
    return ret;
}

/*
 * Make all directory elements of a path
 *
 * Note: 
 * mode should apply to final directory 
 * and others should use the parent mode - this is not important
 * for our use case, so we make every dir that we create same mode.
 *
 * Note:
 * We do not normalize the path - our use case does not require it.
 */
int makepath(const char *path, mode_t mode) {
    int ret = 0;
    char *tmp = nullptr;
    size_t plen = 0;
    //mode_t pmode = 0;

    if (path == nullptr || path[0] == '\0' || path[0] == '/') {
        goto exit;
    }

    tmp = strdup(path);
    if (tmp == nullptr) {
        perror(nullptr);
        ret = -1; 
        goto exit;
    }

    plen = strlen(tmp);
    if (tmp[plen-1] == '/') {
        tmp[plen-1] = '\0';
    }

    for (char *ptr = tmp +1; *ptr ; ptr++) {
        if (*ptr == '/') {
            *ptr = '\0';
            ret = makedir(tmp, mode);
            if (ret != 0) {
                goto exit;
            }
            *ptr = '/';
        }
    } 

    /*
     * final dir
     */
    ret = makedir(tmp, mode);
    if (ret != 0) {
        goto exit;
    }

exit:
    if (tmp != nullptr) {
        free((void *)tmp);
    }
    return ret;
}
