// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Strip out leading/trailing whitespce
 * - in place
 */
#include <ctype.h>
#include <string.h>

void strip_whitespace(char *str) {
    if (str == NULL || *str == '\0') {
        return;
    }

    size_t start = 0;
    size_t end = strlen(str) - 1;

    /*
     * first non-whitespace character
     */
    while (str[start] && isspace((unsigned char)str[start])) {
        start++;
    }

    /*
     * last non-whitespace character
     */
    while (end >= start && isspace((unsigned char)str[end])) {
        end--;
    }

    /*
     * Shift to front
     */
    size_t idx = 0;
    for (idx = start; idx <= end; idx++) {
        str[idx - start] = str[idx];
    }

    str[end-start] = '\0';
}

char *trim_string(char *str, size_t max_len) {
    /*
     * Trim space from str.
     * - add null after last non-whitespace
     * - return pointer to the first non-nonwhitespace
     */
    char *end = nullptr;
    char *ptr = str;

    /*
     * first non-space
     * - NB (ptr - str) >= 0 by construction
     */
    while (isspace((unsigned char)*ptr)) {
        size_t ps_diff = (size_t)(ptr - str);
        if (ps_diff >= max_len) {
            break;
        }
        ptr++;
    }
    if (*ptr == '\0') {
        return ptr;
    }

    /*
     * last non-space
     */
    end = ptr + strlen(ptr) - 1;
    while (end > ptr && isspace((unsigned char)*end)) {
        end--;
    }
    end[1] = '\0';
    return ptr;
}

