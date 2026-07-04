// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

/**
 * Make a duplicate copy
 */
int array_str_dup(Array_str *arr_1, Array_str *arr_2) {

    if (!arr_1 || !arr_2) {
        return -1;
    }

    array_str_free(arr_2);

    /*
     * nothing in arr_1
     */
    if (!arr_1->rows || arr_1->num_rows == 0) {
        return 0;
    }

    /*
     * alloc arr_2
     */
    if (array_str_resize(arr_1->num_rows, arr_2) != 0) {
        msg(MSG_ERR, "  sd-boot: memory allocation error\n");
        return -1;
    }

    /*
     * Duplicate elements
     */
    for (size_t i = 0; i < arr_1->num_rows; i++) {
        if (arr_1->rows[i]) {
            char *tmp = strdup(arr_1->rows[i]);
            if (!tmp) {
                perror("strdup fail");
                return -1;
            }

            arr_2->rows[i] = tmp;
            arr_2->row_len[i] = arr_1->row_len[i];

        } else {
            arr_2->rows[i] = nullptr;
            arr_2->row_len[i] = 0;
        }
    }

    arr_2->num_rows_used = arr_1->num_rows_used;

    return 0;
}

