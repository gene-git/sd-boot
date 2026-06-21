// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "sd-boot-utils.h"

/**
 * Add terminal nullptr to list if not present
 * - If last element is not a null then add 1 element and set to null
 */
int array_str_null_terminate(Array_str *arr) {

    /*
     * Check if last element is null terminated
     */
    bool is_null_terminated = false;

    /*
     * Check for null terminated element after last non-null element
     * - handles the case : (x, y, null, z) where z could be ignored 
     *   by things searching to find a null. Our code is always explcicit
     *   but we're careful for other lib functions such as execvpe() that 
     *   walk through argv/envp to find the last elem.
     */
    for (size_t i = 0; i < arr->num_rows; i++) {
        if (arr->rows[i]) {
            continue;
        }
        if (!arr->rows[i]) {
            is_null_terminated = true;
            break;
        }
    }

    if (is_null_terminated) {
        return 0;
    }

    if (array_str_resize(arr->num_rows + 1, arr) != 0) {
        return -1;
    }

    arr->rows[arr->num_rows - 1] = nullptr;

    return 0;
}
