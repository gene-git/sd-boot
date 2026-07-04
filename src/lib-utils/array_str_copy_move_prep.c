// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"


/**
 * Prepares arr_2 for copy or move rows :
 *
 * Returns:
 *  1 = ready to go
 *  0 = nothing to do (arr_1 has nothing)
 * -1 = error (memory)
 */
int array_str_copy_move_prep(Array_str *arr_1, Array_str *arr_2, size_t *first_new_row_p) {

    if (!arr_1 || !arr_2) {
        return -1;
    }

    /*
     * Scan to find start at first null or after the last non-null (if not null terminaed)
     * truncate if needed.
     */
    size_t first_new_row = arr_2->num_rows;
    for (size_t i = 0; i < arr_2->num_rows; i++) {
        if (!arr_2->rows[i]) {
            first_new_row = i;
            break;
        }
    }
    *first_new_row_p = first_new_row;

    /*
     * Clean up in case if arr_2 has non-null rows after a null row
     */
    if (first_new_row < arr_2->num_rows) {
        if (array_str_resize(first_new_row, arr_2) != 0) {
            return -1;
        }
    }

    /*
     * Check if anything in arr_1 to copy
     */
    if (!arr_1->rows || arr_1->num_rows == 0) {
        return 0;
    }

    /*
     * number of non-null rows to be appended
     */
    size_t rows_to_append = 0;
    for (size_t i = 0; i < arr_1->num_rows; i++) {
        if (arr_1->rows[i]) {
            rows_to_append++;
        }
    }

    if (rows_to_append == 0) {
        return 0;
    }


    /*
     * Allocate whats needed
     */
    size_t num_rows = first_new_row + rows_to_append;
    if (num_rows > arr_2->num_rows) {
        if (array_str_resize(num_rows, arr_2) != 0) {
            msg(MSG_ERR, "  sd-boot: memoray allocation fail\n");
            return -1;
        }
    }

    /*
     * All good to copy or move
     */
    return 1;
}
