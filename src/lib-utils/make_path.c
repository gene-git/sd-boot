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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-utils.h"


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

    if (!path || path[0] == '\0' || strcmp(path, "/") == 0) {
        return 0;
    }

    char *tmp = strdup(path);
    if (!tmp) {
        perror("memory allocation error");
        return -1;
    }

    int ret = 0;

    for (char *ptr = tmp + 1; *ptr != '\0'; ptr++) {

        if (*ptr == '/') {
            *ptr = '\0';
            ret = makedir(tmp, mode);
            *ptr = '/';

            if (ret != 0) {
                goto exit;
            }
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
    free(tmp);
    return ret;
}
