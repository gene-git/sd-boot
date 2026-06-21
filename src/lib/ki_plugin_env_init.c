// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Create KERNEL_INSTALL_PLUGINS from list of active plugins
 * - msut be called after get_active_plugins() has initialized conf->active_plugins;
 * - save result in conf->env_active_plugins
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-utils.h"

int ki_plugin_env_init(SdBoot *conf) {

    int ret = 0;
    char *var = nullptr;
    Array_str *plugins = &conf->active_plugins;

    /*
     * env KERNEL_INSTALL_PLUGINS
     */
    const char *env_name = "KERNEL_INSTALL_PLUGINS=";
    size_t bytes = strlen(env_name) + 1;

    for (size_t i = 0; i < plugins->num_rows; i++) {
        bytes += plugins->row_len[i] + 1;
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

    for (size_t i=0; i < plugins->num_rows; i++) {
        if (plugins->rows[i]) {
            strlcat(var, plugins->rows[i], bytes);
            strlcat(var, " ", bytes);
        }
    }

    ret = array_str_new(1, &conf->env_active_plugins);
    if (ret != 0) {
        goto exit;
    }
    conf->env_active_plugins.rows[0] = var;
    var = nullptr;

    array_str_refresh_row_len(&conf->env_active_plugins);

exit:
    if (var) {
        free((void *)var);
    }
    return ret;
}

