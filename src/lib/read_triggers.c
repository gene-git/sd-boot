// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * pacman's ALPM Hooks contain lists of triggers. When a trigger changes then
 * pacman writes it to the stdin of the "Action" executable (that's us) one trigger per line.
 *
 * Read and return the list of all triggers from stdin. 
 * Thay are either a path or a package name.
 * Return the list of triggers 
 */
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>

#include "sd-boot.h"

enum ReadInfo {
    READ_TIMEOUT = 10000,
    CHUNK = 32,
};

static int process_one_line(char *row, size_t row_size, Array_str *arr) {
    /*
     * - Update mem allocated if needed (increase only)
     * - save the data
     */
    int ret = 0;
    size_t width = 0;
    size_t n_row = 0;

    n_row = arr->num_rows_used++;
    if (arr->num_rows_used > arr->num_rows) {
        ret = array_str_resize(arr->num_rows + CHUNK, arr);
        if (ret != 0) {
            goto exit;
        }
    }
    width = strnlen(row, row_size);
    arr->rows[n_row] = strdup(row);
    arr->row_len[n_row] = width;

exit:    
    return ret;
}

int read_triggers(Array_str *arr) {
    /*
     * List of lines returned in arr->rows
     * - arr->num_rows 
     * - use timeout so we dont' get stuck waiting indefinitely.
     */
    struct pollfd pfd = {};
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    int timeout_ms = READ_TIMEOUT;
    int poll_ret = 0;
    int status = 0;
    int ret = 0;
    char row[PATH_MAX];

    /*
     * We don't know how many lines we'll read so allocate in chunks
     */
    size_t num_rows = CHUNK;
    ret = array_str_new(num_rows, arr);
    if (ret != 0) {
        msg(MSG_ERR, "sd-boot: read triggers memory alloc fail\n");
        return -1;
    }
    bool keep_reading = true; 

    while (keep_reading) {
        poll_ret = poll(&pfd, 1, timeout_ms);
        if (poll_ret > 0 && ((unsigned short)pfd.revents & (unsigned short)POLLIN)) {
            /*
             * Read one line from stdin
             */
            ret = read_one_line_fd(pfd.fd, row, sizeof(row));
            switch (ret) {
                default:
                case 0:
                    ret = process_one_line(row, sizeof(row), arr);
                    if (ret < 0) {
                        status = -1;
                        goto exit;
                    }
                    break;

                case 1:
                    keep_reading = false;
                    break;

                case -1:
                    keep_reading = false;
                    status = -1;
                    break;
            }
        } else {
            keep_reading = false;
        }
    }

    /*
     * resize to what was used
     */
    if (arr->num_rows_used != arr->num_rows) {
        ret = array_str_resize(arr->num_rows_used, arr);
        if (ret != 0) {
            msg(MSG_ERR, "sd-boot: memory error\n");
            status = -1;
            goto exit;
        }
    }

exit:
    return status;
}

