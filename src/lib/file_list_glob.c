// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Return a List of files matching glob pattern.
 *
 * Returns list of pointers to strings - caller responsible for freeing the mem.
 * using array_str_free().
 */
#include <glob.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

int file_list_glob(const char *pattern, Array_str *files) {
    int ret = 0;
    glob_t gstruct = {};

    if (pattern == nullptr || pattern[0] == '\0') {
        goto exit;
    }

    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    ret = glob(pattern, 0, nullptr, &gstruct);
    if (ret == 0) {
        files->num_rows = gstruct.gl_pathc;
        ret = array_str_new(files->num_rows, files);
        if (ret != 0) {
            ret = -1;
            goto exit;
        }

        for (size_t i = 0; i < files->num_rows; i++) {
            files->rows[i] = strdup(gstruct.gl_pathv[i]);
            if (files->rows[i] == nullptr) {
                ret = -1;
                array_str_free(files);
                goto exit;
            }
            files->row_len[i] = strlen(files->rows[i]);
        }
    }

exit:
    globfree(&gstruct);
    return ret;
} 

