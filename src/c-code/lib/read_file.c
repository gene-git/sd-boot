// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * read config file
 * Contents are comments (# ...)
 * key = value pairs
 */
#include <errno.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

enum Sizes { NUM_ROWS = 64, };

char *read_file_first_row(const char *path) {
    /*
     * Read file
     * - ignore empty, comments
     * - return first non empty row
     * - caller must free memory returned
     * 
     * 0 = success
     * 1 = no file available (fopen failed) empty file
     * -1 = error
     */
    int ret = 0;
    char *row = nullptr;
    Array_str arr = {};

    ret = read_file(path, &arr);
    if (ret == 1) {
        // no such file 
        goto exit;
    }

    if (ret < 0) {
        msg(MSG_ERR, "sd-boot: Failed reading file: %s\n", path);
        goto exit;
    }

    if (arr.num_rows > 0) {
        row = arr.rows[0];
        arr.rows[0] = nullptr;
    }

exit:
    array_str_free(&arr);
    return row;
}

void close_file(FILE *file) {
    if (file != nullptr && fclose(file) != 0) {
        perror(nullptr);
    }
}

int read_file(const char *path, Array_str *arr) {
    /*
     * Read a text file
     * - comments, empty lines igmored
     * Args:
     *  path = file with key/val data
     *
     * Returns:
     *  Array_str filled out - the array is in arr->rows;
     * 
     * 0 = success
     * 1 = no file available (fopen failed)
     * -1 = error
     *
     * Note - all our files are small number of rows, and max cols of PATH_PAX +so we alloc 64 rows
     * after that we realloc as needed - if files coule be longer we should
     * realloc rows up by 64 (or other appropriate number) at a time.
     */
    int ret = 0;
    char row[ROW_MAX] = {'\0'};

    /*
     * Return 1 if doesn't exist.
     * -1 for an error
     */
    FILE *file = fopen(path, "r");
    if (file == nullptr) {
        /*
         * 1 = no such file
         */
        if (errno == ENOENT) { // NOLINT(misc-include-cleaner)
            return 1;
        }
        return -1;
    }

    /*
     * Allocate a chunk of rows, then grow or shrink as needed.
     */
    if (array_str_new(NUM_ROWS, arr) != 0) {
        ret = -1;
        goto exit;
    }

    size_t width = 0;
    size_t n_row = 0;
    char *ptr = nullptr;
    arr->num_rows_used = 0;

    while (fgets(row, sizeof(row), file)) {
        /*
         * Skip comments, empty
         */
        ptr = trim_string(row, ROW_MAX);
        if (ptr[0] == '#' || ptr[0] == '\n' || ptr[0] == '\0') {
            continue;
        }

        /*
         * ensure enough mem for new row
         */
        arr->num_rows_used += 1;
        if (arr->num_rows_used > arr->num_rows) {
            ret = array_str_resize(arr->num_rows_used, arr);
            if (ret != 0) {
                goto exit;
            }
        }

        /*
         * save it
         */
        n_row = arr->num_rows_used - 1;
        width = strnlen(ptr, PATH_MAX);
        arr->row_len[n_row] = width;
        arr->rows[n_row] = strdup(ptr);
        if (arr->rows[n_row] == nullptr) {
            ret = -1;
            goto exit;
        }
    }

    /*
     * final resize to what was used.
     */
    if (arr->num_rows_used != arr->num_rows) {
        ret = array_str_resize(arr->num_rows_used, arr);
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    close_file(file);
    return ret;
}
