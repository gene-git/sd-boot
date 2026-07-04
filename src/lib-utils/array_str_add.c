// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Append a new row with the provided string.
 */
#include <stdio.h>
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

/**
 * Add one string to arr->rows[]
 * Input:
 *  string
 *
 * Returns 0 on success.
 *  
 */
int array_str_add_string(const char *string, Array_str *arr) {

    if (!arr || !string) {
        return -1;
    }

    size_t n_row = arr->num_rows;

    int ret = array_str_resize(n_row + 1, arr);
    if (ret != 0) {
        msg(MSG_ERR, " sd-boot: memory alloc error\n");
        return ret;
    }

    char *tmp_str = strdup(string);
    if (!tmp_str) {
        perror("strip fail");

        /*
         * restore original 
         */
        (void)array_str_resize(n_row, arr);
        return -1;
    }

    arr->rows[n_row] = tmp_str;
    arr->row_len[n_row] = strlen(tmp_str);

    return 0;
}

