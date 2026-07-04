// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Misc tools
 */
#include <limits.h>
#include <stdlib.h>


bool string_to_long(const char *str, long *long_val) {

    if (!str || !long_val) {
        return false;
    }

    char *endptr = nullptr;

    /*
     *  Base 0 => decimal, octal (0755) or hex (0x1A)
     */
    long val = strtol(str, &endptr, 0);
    if (endptr == str || *endptr != '\0') {
        return false;
    }
    *long_val = val;
    return true;
}

bool string_to_int(const char *str, int *int_val) {

    if (!str || !int_val) {
        return false;
    }

    long long_val = 0;

    if (!string_to_long(str, &long_val)) {
        return false;
    }
    if (long_val < INT_MIN || long_val > INT_MAX) {
        return false;
    }

    *int_val = (int)long_val;
    return true;
}
