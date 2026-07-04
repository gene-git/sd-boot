// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Strip out leading/trailing whitespce
 * - in place
 */
#include <ctype.h>
#include <string.h>


/*
 * Trim white space from str.
 * - add null after last non-whitespace
 * - return pointer to the first non-nonwhitespace
 */
char *trim_string(char *str, size_t max_len) {

    if (!str || max_len == 0) {
        return str;
    }
    char *ptr = str;
    size_t remaining = max_len;

    while (remaining > 0 && isspace((unsigned char)*ptr)) {
        if (*ptr == '\0') {
            return ptr;
        }
        ptr++;
        remaining--;
    }

    size_t len = strnlen(ptr, remaining);
    if (len == 0) {
        /*
         * If empty or spaces
         */
        if (max_len > (size_t)(ptr - str)) {
            *ptr = '\0';
        }
        return ptr;
    }

    /*
     * trim any traling whitespace
     */
    char *end = ptr + len - 1;
    while (end > ptr && isspace((unsigned char)*end)) {
        end--;
    }

    end[1] = '\0';

    return ptr;

}

