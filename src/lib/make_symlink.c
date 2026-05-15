// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Make a symlink.
 *
 * linkname -> target
 *
 * Replaces any existing linkname if the target is not the same.
 */
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sd-boot.h"

/*
 * Check whether symlink is neede linkis same as requested
 * returns
 * -1 = error 
 *  0 = all is fine.
 *      need_link true = nothing to do - target matches
 *      need_link true = create new link
 */

static int is_symlink_needed(const char *target, const char *linkname, bool *need_link) {
    int ret = 0;

    struct stat link_stat = {};
    char curr_targ[PATH_MAX] = {'\0'};
    ssize_t bytes_read = 0;
    *need_link = true;

    /*
     * Check linkname
     */
    if (lstat(linkname, &link_stat) == 0) {
        if (S_ISLNK(link_stat.st_mode)) {
            /*
             * Linkname exists
             * - delete if wrong target
             */
            bytes_read = readlink(linkname, curr_targ, PATH_MAX-1);
            if (bytes_read < 0) {
                ret = -1;
                goto exit;
            }
            if (strcmp(curr_targ, target) == 0) {
                *need_link = false;
            } else {
                /*
                 * wrong target
                 */
                ret = unlink(linkname);
                if (ret != 0) {
                    msg(MSG_ERR, "sd-boot: error deleting linkname: %s\n", linkname);
                    ret = -1;
                    goto exit;
                }
            }
        } else {
            /*
             * not a symlink
             */
            msg(MSG_ERR, "sd-boot: linkname exists but not a symlink: %s\n", linkname);
            ret = -1;
            goto exit;
        }
    }
exit:
    return ret;
}

int make_symlink(const char *target, const char *linkname) {
    int ret = 0;

    if (target == nullptr || linkname == nullptr) {
        msg(MSG_ERR, "sd-boot bad input to make symlink\n");
        ret = -1;
        goto exit;
    }

    bool need_link = true;
    ret = is_symlink_needed(target, linkname, &need_link);
    if (ret != 0) {
        ret = -1;
        goto exit;
    }

    if (need_link) {
        ret = symlink(target, linkname);
        if (ret != 0) {
            perror(nullptr);
            ret = -1;
            goto exit;
        }
    }

exit:
    return ret;
}

