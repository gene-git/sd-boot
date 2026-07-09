// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
#include <stddef.h>
#include <string.h>

#include "sd-boot-utils.h"

/**
 * Add terminal nullptr to list if not present
 * - If last element is not a null then add 1 element and set to null
 */
int array_str_null_terminate(Array_str *arr) {

    if (!arr) {
        return -1;
    }

    /*
     * CHeck if null terminated
     */
    for (size_t i = 0; i < arr->num_rows; i++) {
        if (arr->rows[i] == NULL) {
            return 0;
        }
    }

    /*
     * null terminate
     * - array_str_resize nulls the new tail pointer and sets row_len to 0.
     */
    if (array_str_resize(arr->num_rows + 1, arr) != 0) {
        return -1;
    }

    return 0;
}
