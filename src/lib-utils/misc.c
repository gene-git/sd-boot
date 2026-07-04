// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Misc tools
 */
#include <limits.h>
#include <linux/limits.h>
#include <string.h>


void strip_file_extension(char *filename, const char *ext) {
    /*
     * Strip extenion by adding '\0' at the dot
     */
    char *dot = strrchr(filename, '.');

    if (dot && strncmp(dot, ext, PATH_MAX) == 0) {
        *dot = '\0';
    }
}

