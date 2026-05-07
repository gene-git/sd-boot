// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Remove a file
 * - if file exists its not an error
 *   basically "rm -f"
 */
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdio.h>

int remove_file(char *path) {
    /*
     * Remove a file if it exists
     */
    int ret = remove(path);
    if (ret != 0) {
        if (errno == ENOENT) {
            ret = 0;
        } else {
            perror("Error deleting file");
        } 

    }

    return ret;
}
