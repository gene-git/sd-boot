// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Identify the first argument to kernel-install which is the operation to perform
 * - must be one of: 
 *   add, remove, inspect, add-all, list
 *   At moment, sd-boot uses add/remove 
 */
#include <string.h>

#include "sd-boot.h"

int kernel_install_oper(char *oper) {
    /*
     * Valid args are add or remove
     */
    if (strncmp(oper, "add", ARG_SZ) == 0) {
        return ADD;
    }

    if (strncmp(oper, "remove", ARG_SZ) == 0) {
        return REMOVE;
    }

    if (strncmp(oper, "inspect", ARG_SZ) == 0) {
        return INSPECT;
    }

    if (strncmp(oper, "add-all", ARG_SZ) == 0) {
        return ADD_ALL;
    }

    if (strncmp(oper, "list", ARG_SZ) == 0) {
        return LIST;
    }
    return BAD;
}


