// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Return list of all active plugins.
 * This is all plugins without any in the skip list: conf->skip_kernel_plugins
 *
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"

/*
 * Return list of active plugins.
 * - plugins should be empty.
 */

/*
 * When checking the plugin file name against list of skips
 * need to prepent root when in test mode
 */
static bool skip_this_one(SdBoot *conf, const char *name, Array_str *skips) {
    char *this_name = nullptr;
    size_t root_len = 0;

    if (!skips || !skips->rows || skips->num_rows == 0) {
        return false;
    }

    /*
     * Point to first "/" after the test root part
     */
    root_len = strlen(conf->root);

    for (size_t i = 0; i < skips->num_rows; i++) {
        if (!skips->rows[i]) {
            continue;
        }

        /*
         * Adjust path to ignore the test root prefix
         */
        if (conf->test) {
            this_name = (char *)name + root_len - 1;
        } else {
            this_name = (char *)name;
        }

        if (strcmp(this_name, skips->rows[i]) == 0) {
            return true;
        }
    }
    return false;
}

/*
 * Get:
 * Must call get_all_plugins() before calling this function to ensure
 * that conf->all_plugins is initialized.
 *  conf->all_plugins
 *  conf->active_plugins
 *
 *  If we don't need to keep all_plugins, we can just move the memory to active_plugins.
 */
int get_active_plugins(SdBoot *conf) {

    int ret = 0;
    size_t num_plugins = 0;
    Array_str *all_plugins = &conf->all_plugins;
    Array_str *active_plugins = &conf->active_plugins; 

    /*
     * Nothing being skipped
     */
    if (!conf->skip_kernel_plugins.rows && conf->skip_kernel_plugins.num_rows == 0) {
        ret = array_str_dup(all_plugins, active_plugins);
        return ret;
    }

    /*
     * Skip some
     */
    ret = array_str_resize(all_plugins->num_rows, active_plugins);
    if (ret != 0) {
        goto exit;
    }
    char *tmp = nullptr;

    /*
     * Copy or skip
     */
    for (size_t i = 0; i < all_plugins->num_rows; i++) {

        const char *this_one = all_plugins->rows[i];
        if (!this_one) {
            continue;
        }

        if (!skip_this_one(conf, this_one, &conf->skip_kernel_plugins)) {
            tmp = strdup(all_plugins->rows[i]);
            if (!tmp) {
                ret = -1;
                goto exit;
            }
            active_plugins->rows[num_plugins++] = tmp;
        }
    }

    ret = array_str_resize(num_plugins, active_plugins);
    if (ret != 0) {
        goto exit;
    }

    array_str_refresh_row_len(active_plugins);

exit:
    return ret;
}

