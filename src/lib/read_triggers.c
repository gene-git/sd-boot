// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *  * pacman's ALPM Hooks contain lists of triggers. When a trigger changes then
 * pacman writes it to the stdin of the "Action" executable (that's us) one trigger per line.
 *  
 * Read and return the list of all triggers from stdin. 
 * Thay are either a path or a package name.
 * Return the list of triggers 
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"

enum ReadInfo { CHUNK = 32 };

/*
 * - Update mem allocated if needed (increase only)
 * - save the data
 */
static int process_one_line(const char *row, ssize_t len, Array_str *arr) {
    int ret = 0;
    size_t n_row = arr->num_rows_used++;

    if (arr->num_rows_used > arr->num_rows) {
        ret = array_str_resize(arr->num_rows + CHUNK, arr);
        if (ret != 0) {
            return ret;
        }
    }

    arr->rows[n_row] = strdup(row);
    if (!arr->rows[n_row]) {
        return -1;
    }
    arr->row_len[n_row] = (size_t)len;
    return 0;
}

/*
 * List of lines read from stdin are returned in arr->rows
 * - arr->num_rows
 * - use timeout so we dont' get stuck waiting indefinitely.
 */
int read_triggers(Array_str *arr) {
    int status = 0;
    size_t num_rows = CHUNK;

    /*
     * We must read all lines and we dont know how many.
     * Allocate in chunks to reduce malloc() calls
     */
    int ret = array_str_new(num_rows, arr);
    if (ret != 0) {
        msg(MSG_ERR, " ! sd-boot: read triggers memory alloc fail\n");
        return -1;
    }

    /*
     * Use buffered stream for stdin
     */
    FILE *stream = fdopen(STDIN_FILENO, "r");
    if (!stream) {
        return -1;
    }

    char *line = nullptr;
    size_t cap = 0;
    ssize_t len = 0;

    while ((len = getline(&line, &cap, stream)) != -1) {
        /*
         *  Strip trailing newline character
         */
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        if (process_one_line(line, len, arr) < 0) {
            status = -1;
            break;
        }
    }

    free(line);

    ret = array_str_resize(arr->num_rows_used, arr);
    if (ret != 0) {
        status = -1;
    }

    if (ferror(stream)) {
        status = -1;
    }

    if (fclose(stream) != 0) {
        status = -1;
    }

    return status;
}

