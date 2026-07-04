// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Dynamic string
 * Handles strings up to MEM_MAX ~ 100 MB
 */
#include <stdlib.h>
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

enum Constants {
    MEM_MAX = 100 * 1024 * 1024,
};


int dynamic_str_alloc(size_t num, Dynamic_str *str) {
    /*
     * Free
     */
    if (num == 0) {
        if (str->num_alloc != 0 || str->bytes) {
            free(str->bytes);
            str->bytes = nullptr;
            str->num_alloc = 0;
            str->num_used = 0;
        }
        return 0;
    }

    /*
     * Alloc
     */
    if (num > MEM_MAX) {
        msg(MSG_ERR, "  sd-boot: dynamic_str_alloc: mem allocation too big\n");
        return -1;
    }

    if (num != str->num_alloc) {

        if (str->num_alloc == 0 || !str->bytes) {
            str->bytes = malloc(num);
            if (!str->bytes) {
                msg(MSG_ERR, "  sd-boot: dynamic_str_alloc: mem allocation error\n");
                return -1;
            }
            str->num_alloc = num;

        } else {
            char *tmp = realloc(str->bytes, num);
            if (!tmp) {
                msg(MSG_ERR, "  sd-boot: dynamic_str_alloc: mem allocation error\n");
                return -1;
            }
            str->bytes = tmp;
            str->num_alloc = num;
        }
    }

    return 0;
}

void dynamic_str_free(Dynamic_str *str) {
    (void) dynamic_str_alloc(0, str);
}
