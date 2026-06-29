// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Get trigger info.
 *
 * Triggers are read from stdin and separated by type:
 * - packages
 * - paths
 *
 * A path is recognized as containing at least 1 "/".
 * A path trigger without a "/" will be treated as a package.
 */
// DEBUG
#include <stdlib.h>
#include <stdbool.h>
#include "sd-boot-msg.h"

#include "sd-boot-utils.h"
#include "sd-boot.h"
#include <stddef.h>

static int t_allocate(size_t num_pkgs, size_t num_paths, TriggerInfo *all) {
    int ret = 0;

    ret = array_str_resize(num_pkgs, &all->pkgs);
    if (ret != 0) {
        goto exit;
    }

    ret = array_str_resize(num_paths, &all->paths);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}

int get_triggers(TriggerInfo *trigs) {
    int ret = 0;
    size_t num_all = 0;
    size_t num_pkgs = 0;
    size_t num_paths = 0;
    Array_str all = {};

    ret = read_triggers(&all);
    if (ret != 0) {
        goto exit;
    }

    num_all = all.num_rows;
    if (num_all == 0) {
        goto exit;
    }

    /*
     * Allocate to max
     */
    ret = t_allocate(num_all, num_all, trigs);
    if (ret != 0) {
        goto exit;
    }

    /*
     * split
     * - paths dont have leading "/" - fix it here.
     */
    Array_str *pkgs = &trigs->pkgs;
    Array_str *paths = &trigs->paths;

    // debug
    bool debug = false;
    if (getenv("SDB_DEB")) {
        debug = true;
        msg(MSG_ERR, " SDB_DEB set\n");
    } 

    for (size_t i = 0; i < num_all; i++) {
        if (debug) {
            msg(MSG_ERR, "  %s\n", all.rows[i]);
        }
        if (trigger_is_path((const char *)all.rows[i])) {

            ret = path_add_slash(all.rows[i], &paths->rows[num_paths++]);
            if (ret != 0) {
                goto exit;
            }

        } else {

            pkgs->rows[num_pkgs++] = all.rows[i];
            all.rows[i] = nullptr;

        }
    }

    /*
     * Adjust allocations
     */
    ret = t_allocate(num_pkgs, num_paths, trigs);
    if (ret != 0) {
        goto exit;
    }

exit:
    array_str_free(&all);
    return ret;
}
