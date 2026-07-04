// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Remove a file
 * - if file exists its not an error
 *   basically "rm -f"
 */
#include <errno.h>
#include <stdio.h>


int remove_file(const char *path) {

    if (!path || path[0] == '\0') {
        return -1;
    }

    int ret = remove(path);
    if (ret != 0) {
        if (errno == ENOENT) {          // NOLINT(misc-include-cleaner)
            ret = 0;
        } else {
            perror("Error deleting file");
        }
    }

    return ret;
}
