// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Return list of all plugins *.install found in:
 *
 * - /usr/lib/kernel/install.d/
 * - <etc-root>/etc/kernel/install.d/.
 *
 * Where <etc-root> is:
 * -  "/" in production mode 
 * - root of the testing tree in testing mode.
 *   For example /path/to/Testing/__root__
 *
 * Note that usr plugins are taken from /usr/lib/... 
 *
 */
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>

#include "sd-boot.h"

int get_plugin_list(char *root, Array_str *plugins) {
    /*
     * Env will be all the  *.install files from 
     * - usr plugins from <root>/usr/lib/kernel/install.d
     * - etc plugins from "<root>/etc/kernel/install.d
     */
    int ret = 0;
    char pattern[PATH_MAX] = {};
    Array_str etc_plugins = {};

    /*
     * usr plugins
     */
    ret = array_str_free(plugins);
    if (ret != 0) {
        goto exit;
    }

    if (snprintf(pattern, PATH_MAX, "%s%s", root, "usr/lib/kernel/install.d/*.install") < 0) {
        ret = -1;
        goto exit;
    }

    ret = file_list_glob(pattern, plugins);
    if (ret != 0) {
        goto exit;
    }

    /*
     * etc plugins
     */
    if (snprintf(pattern, PATH_MAX, "%s%s", root, "etc/kernel/install.d/*.install") < 0) {
        ret = -1;
        goto exit;
    }

    ret = file_list_glob(pattern, &etc_plugins);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Combine them
     */
    ret = array_str_move(&etc_plugins, plugins);
    if (ret != 0) {
        goto exit;
    }

exit:
    array_str_free(&etc_plugins);
    return ret;
}

