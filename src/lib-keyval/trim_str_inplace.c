// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Strip out leading/trailing whitespce
 * - in place - return pointer into the string of first non-space
 * - string is modified.
 */
#include <ctype.h>
#include <string.h>

#include "sd-boot-keyval.h"

char *trim_str_inplace(char *str) {

    if (!str) {
        return str;
    }

    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') {
        return str;
    }

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    return str;
}


