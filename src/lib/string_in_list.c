// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Check if the string 'name' is in a list of strings.
 */

#include <stdbool.h>
#include <string.h>

bool string_in_list(char *name, size_t num_names, char **names) {
    /*
     * Return true if name in names
     * else false
     */
    if (name == nullptr || names == nullptr) {
        return false;
    }

    for (size_t i = 0; i < num_names; i++) {
        if (strcmp(name, names[i]) == 0) {
            return true;
        }
    }
    return false;
}
