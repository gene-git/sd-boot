// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"


/**
 * Append copies of all (non-null) rows from arr_1 onto arr_2
 * They are appended to arr_2 at the first null row or after
 * the last non-null row.
 */
int array_str_copy_rows(Array_str *arr_1, Array_str *arr_2) {

    size_t first_new_row = arr_2->num_rows;

    /*
     * returns
     *   -1 = error
     *    0 = nothing to do
     *    >0 - keep going
     */
    int prep_status = 0;
    prep_status = array_str_copy_move_prep(arr_1, arr_2, &first_new_row);

    if (prep_status < 0) {
        return -1;
    }

    if (prep_status == 0) {
        return 0;
    }

    /*
     * Copy 
     */
    size_t num_final = first_new_row;
    for (size_t i = 0; i < arr_1->num_rows; i++) {
        if (arr_1->rows[i]) {
            char *tmp = strdup(arr_1->rows[i]);
            if (!tmp) {
                msg(MSG_ERR, "  sd-boot: strdup failed\n");
                return -1;
            }
            arr_2->rows[num_final] = tmp;
            arr_2->row_len[num_final] = arr_1->row_len[i];

            num_final++;
        }
    }
    arr_2->num_rows_used += arr_1->num_rows_used;

    return 0;
}
