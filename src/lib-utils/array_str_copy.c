// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
#include <stddef.h>
#include <string.h>

#include "sd-boot-utils.h"

/**
 * Make a duplicate copy (since we dont do ref counting).
 */
int array_str_dup(Array_str *arr_1, Array_str *arr_2) {
    if (!arr_1->rows || arr_1->num_rows == 0) {
        return 0;
    }

    if (array_str_resize(arr_1->num_rows, arr_2) != 0) {
        return -1;
    }

    /*
     * Duplicate elements
     */
    for (size_t i = 0; i < arr_1->num_rows; i++) {
        if (arr_1->rows[i]) {
            arr_2->rows[i] = strdup(arr_1->rows[i]);
            arr_2->row_len[i] = strlen(arr_2->rows[i]);
        }
    }

    return 0;
}

/**
 * Append copies of all (non-null) rows from arr_1 onto arr_2
 */
int array_str_copy_rows(Array_str *arr_1, Array_str *arr_2) {

    if (!arr_1->rows || arr_1->num_rows == 0) {
        return 0;
    }

    /*
     * Start at first null or after the last non-null (if not null terminaed)
     */
    size_t first_new_row = arr_2->num_rows;
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

    size_t num_final = first_new_row;
    for (size_t i = 0; i < arr_1->num_rows; i++) {
        if (arr_1->rows[i]) {
            arr_2->rows[num_final] = strdup(arr_1->rows[i]);
            if (!arr_2->rows[num_final]) {
                return -1;
            }
            arr_2->row_len[num_final] = strlen(arr_2->rows[num_final]);

            num_final++;
        }
    }
    if (array_str_resize(num_final, arr_2) != 0) {
        return -1;
    }
    return 0;
}
