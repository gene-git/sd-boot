// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
#include <stddef.h>

#include "sd-boot-utils.h"

/**
 * Move all rows from one arr_1 and append to arr_2
 */
int array_str_move(Array_str *arr_1, Array_str *arr_2) {

    if (arr_1->num_rows == 0) {
        return 0;
    }

    /*
     * Start at first null or after the last non-null (if not null terminaed)
     */
    size_t first_new_row = 0;
    for (size_t i = 0; i < arr_2->num_rows; i++) {
        if (arr_2->rows[i]) {
            first_new_row = i + 1;
            continue;
        }
        if (!arr_2->rows[i]) {
            first_new_row = i;
            break;
        }
    }

    size_t num_rows = first_new_row + arr_1->num_rows;

    if (array_str_resize(num_rows, arr_2) != 0) {
        return -1;
    }

    /*
     * Move non-null rows from arr_1 to arr_2.
     */
    size_t num_rows_final = first_new_row;
    for (size_t i = 0; i < arr_1->num_rows; i++) {

        if (arr_1->rows[i]) {
            arr_2->rows[num_rows_final] = arr_1->rows[i];
            arr_2->row_len[num_rows_final] = arr_1->row_len[i];

            arr_1->rows[i] = nullptr;
            arr_1->row_len[i] = 0;

            num_rows_final++;
        }
    }

    /*
     * tidy up memory 
     */
    if (array_str_resize(num_rows_final, arr_2) != 0) {
        return -1;
    }

    array_str_free(arr_1);
    return 0;
}
