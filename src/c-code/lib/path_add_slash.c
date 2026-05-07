// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Trigger paths come without a leading "/"
 *
 * This makes a copy and adds the "/" 
 */
#include <stdlib.h>
#include <string.h>

int path_add_slash(char *path, char **path_p) {
    if (path == nullptr || path_p == nullptr) {
        return -1;
    }
    if (path[0] == '/') {

        *path_p = strdup(path);

    } else {
        size_t len = strlen(path);
        size_t size = len + 2;
        *path_p = (char *)calloc(size, sizeof(char));
        if (*path_p == nullptr) {
            return -1;
        }
        (*path_p)[0] = '/';
        (void)strncpy(& (*path_p)[1], path, len+1);
    }
   
    return 0; 
}
