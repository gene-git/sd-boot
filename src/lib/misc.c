// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Misc tools
 */
#include <limits.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

void strip_file_extension(char *filename, const char *ext) {
    /*
     * Strip extenion by adding '\0' at the dot
     */
    char *dot = strrchr(filename, '.');

    if (dot != nullptr && strncmp(dot, ext, PATH_MAX) == 0) {
        *dot = '\0';
    }
}

int str_to_int(char *str, int low_value, int high_value) {
    /*
     * Convert str to intger bounded by low_value <= value <= high_value
     * - we allow str to contain non-numbers - if needed can catch this
     *   case via *endptr != '\0'
     * - if want to check errno, then errno must be set to 0 first.
     *   Its sufficient to check for low/high bound on retrurned value.
     */
    int val = 0;

    if (str == nullptr || str[0] == '\0') {
        return val;
    }
    char *endptr = nullptr;
    long num = strtol(str, &endptr, BASE_10);

    if (*endptr == '\0' || endptr == str) {
        /*
         * entire str is valid
         */
        val = (int) num;

    } else {
        /* 
         * a bad number => errno will be ERANGE
         */
        if (num == LONG_MIN) {
            val = low_value;

        } else if (num == LONG_MAX) {
            val = high_value;
        }
    }
    return val;
}
