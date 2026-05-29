// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Dynamic string
 * Handles strings up to MEM_MAX ~ 100 MB
 */
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

enum Constants {
    MEM_MAX = 100 * 1024 * 1024,
};


int dynamic_str_alloc(size_t num, Dynamic_str *str) {
    int ret = 0;
    /*
     * Free
     */
    if (num == 0) {
        if (str->num_alloc != 0) {
            free((void *)str->bytes);
            str->bytes = nullptr;
            str->num_alloc = 0;
            str->num_used = 0;
        }
        return ret;
    }

    /*
     * Alloc
     */
    if (num > MEM_MAX / sizeof(char)) {
        msg(MSG_ERR, "  sd-boot: dynamic_str_alloc: mem allocation too big\n");
        ret = -1;
        goto exit;
    }

    if (num != str->num_alloc) {

        if (str->num_alloc == 0 || str->bytes == nullptr) {
            str->bytes = (char *)calloc(num, sizeof(char));
            if (str->bytes == nullptr) {
                msg(MSG_ERR, "  sd-boot: dynamic_str_alloc: mem allocation error\n");
                ret = -1;
            }
            str->num_alloc = num;

        } else {
            str->bytes = (char *)realloc(str->bytes, num * sizeof(char));
            if (str->bytes == nullptr) {
                msg(MSG_ERR, "  sd-boot: dynamic_str_alloc: mem allocation error\n");
                ret = -1;
                str->num_alloc = 0;
                goto exit;
            }
            str->num_alloc = num;
        }
    }

exit:
    return ret;
}



