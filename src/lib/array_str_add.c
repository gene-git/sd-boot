// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Append a new row with the provided string.
 */
#include <string.h>

#include "sd-boot.h"

int array_str_add(char *string, Array_str *arr) {
    int ret = 0;
    size_t n_row = 0;

    n_row = arr->num_rows;

    if (arr->num_rows == 0) {
        ret = array_str_new(1, arr);
    } else {
        ret = array_str_resize(arr->num_rows + 1, arr);
    }

    if (ret != 0) {
        msg(MSG_ERR, "  sd-boot: memory alloc error\n");
        goto exit;
    }
    arr->rows[n_row] = strdup(string);

exit:
    return ret;

}
