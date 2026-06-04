// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Create environments to use:
 *
 * a) ki_plugins_test_env() 
 *    When running in test mode we want kernel-install to
 *    use the test plugins.
 *
 *    This is achieved by setting the KERNEL_INSTALL_PLUGINS env variable
 *    to the list of plugins to run.
 *
 * This is only used in testing.
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

int ki_plugins_test_env(char *root, Array_str *env) {
    /*
     * Env will be all the  *.install files from 
     * - standard plugins from /usr/lib/kernel/install.d
     * - plugins from "<root>/etc/kernel/install.d
     */
    int ret = 0;
    char *var = nullptr;
    Array_str plugins = {};

    ret = get_plugin_list(root, &plugins);
    if (ret != 0) {
        goto exit;
    }

    /*
     * env KERNEL_INSTALL_PLUGINS
     */
    const char *env_name = "KERNEL_INSTALL_PLUGINS=";
    size_t bytes = strlen(env_name) + 1;

    for (size_t i=0; i < plugins.num_rows; i++) {
        bytes += plugins.row_len[i] + 1;
    }

    var = (char *)calloc(bytes, sizeof(char));
    if (!var) {
        ret = -1;
        goto exit;
    }

    if (snprintf(var, bytes, "%s", env_name) < 0) {
        ret = -1;
        goto exit;
    }

    for (size_t i=0; i < plugins.num_rows; i++) {
        if (plugins.rows[i] != nullptr) {
            strlcat(var, plugins.rows[i], bytes);
            strlcat(var, " ", bytes);
        }
    }

    ret = array_str_new(2, env);
    if (ret != 0) {
        goto exit;
    }
    env->rows[0] = var;
    var = nullptr;
    env->rows[1] = nullptr;

exit:
    array_str_free(&plugins);
    if (var) {
        free((void *)var);
    }
    return ret;
}

