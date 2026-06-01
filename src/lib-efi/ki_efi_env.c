// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Create the environment variables appropriate for kernel-install
 * to install an efi tool.
 *
 * Efi tools are not kernels. kernel-install can still be used to install these
 * and manage the loader entries, but it only works when layout is bls and
 * when plugins are limited to the appropriate subset.
 *
 * In uki layout kernel-install can use a special config directory
 * where the layout is set to bls. This is a bit ugly but kernel install
 * provides limited dynamic controls.
 *
 * We also limit the list of plugins to those the only relevant ones which 
 * are loader entry plugins.
 *
 * See ki_plugins_efi_update_env()
 *    When installing an efi-tool set the plugins to use that are relevant.
 *    Which is only the plugins to manage loader entries. The other plugins
 *    are relevant only for kernels not efi tools.
 */
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

int ki_efi_update_env(SdBoot *conf, Array_str *env) {
    /*
     * Set up the plugin environment passed to kernel-install:
     *
     * - efi tools are not kernels.
     *   limit the plugins to loader entry items only.
     *
     * Caller responsible for free'ing mem in "env"
     */
    int ret = 0;
    Array_str kernel_conf_env = {};

    /*
     * set up special efi plugins environ to use that sets which plugins are permitted.
     */
    ret = ki_plugins_efi_update_env(conf->info.root, env);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    /*
     * If layout is not bls then, make a "copy" (symlinks aside from install.conf)
     *  of /etc/kernel but with layout set to "bls" (in install.conf)
     */
    if (conf->is_uki) {
        ret = ki_make_kernel_conf_bls(conf);
        if (ret != 0) {
            goto exit;
        }

        /*
         * KERNEL_INSTALL_CONF_ROOT
         */
        const char *env_name = "KERNEL_INSTALL_CONF_ROOT";
        size_t buf_len = PATH_MAX + strlen(env_name) + 2;
        char buf[buf_len + 1] = {};

        if (snprintf(buf, buf_len, "%s=%s", env_name, conf->kernel_conf_bls_dir) < 0) {
            ret = -1;
            goto exit;
        }

        ret = array_str_new(2, &kernel_conf_env);
        if (ret != 0) {
            goto exit;
        }
        kernel_conf_env.rows[0] = strdup(buf);
        kernel_conf_env.rows[1] = nullptr;

        /*
         * Append kernel_install to env
         */
        ret = array_str_move(&kernel_conf_env, env);
    }

exit:
    array_str_free(&kernel_conf_env);
    return ret;
}

