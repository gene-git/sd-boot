// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
#include <stddef.h>

#include "sd-boot.h"

/**
 * Move all rows from one arr_1 to arr_2
 * - last element of arr_2 is set to null pointer. (as expected for env, argv etc);
 */
int array_str_move(Array_str *arr_1, Array_str *arr_2) {
    if (arr_1->num_rows == 0) {
        return 0;
    }

    /*
     * First arr_2 row to get from arr_1
     */
    size_t new_row = arr_2->num_rows;
    for (size_t i = arr_2->num_rows - 1; i > 0; i--) {
        if (arr_2->rows[i]) {
            break;
        }
        new_row = i;
    }

    if (array_str_resize(arr_1->num_rows + arr_2->num_rows, arr_2) != 0) {
        return -1;
    }

    /*
     * Move them
     */
    for (size_t i = 0; i < arr_1->num_rows; i++) {
        size_t irow = new_row++;
        arr_2->rows[irow] = arr_1->rows[i];
        arr_1->rows[i] = nullptr;
        arr_2->row_len[irow] = arr_1->row_len[i];
    }

    /*
     * tidy up memory and ensure last row is nullptr
     */
    if (arr_2->rows[new_row - 1]) {
        new_row++;
    }
    if (array_str_resize(new_row, arr_2) != 0) {
        return -1;
    }
    arr_2->rows[new_row - 1] = nullptr;

    array_str_free(arr_1);
    return 0;
}
