// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Create environments to use:
 *
 * b) ki_plugins_efi_update_env()
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

int ki_plugins_efi_update_env(char *test_root, Array_str *env) {
    /*
     * Set up the plugin environment passed to kernel-install:
     *
     * - efi tools are not kernels.
     *   limit the plugins to loader entry items only.
     *
     * Caller responsible for free'ing mem in "env"
     */
    int ret = 0;

    /*
     * KERNEL_INSTALL_PLUGINS=
     */
    const char* var = "KERNEL_INSTALL_PLUGINS=";
    const char *entry_one = "/usr/lib/kernel/install.d/90-loaderentry.install";
    char entry_two[PATH_MAX] = {};
    char *env_var = nullptr;

    if (env == nullptr) {
        ret = -1;
        goto exit;
    }

    if (snprintf(entry_two, PATH_MAX, " %s/%s", test_root,
                "etc/kernel/install.d/95-sd-boot-loaderentry-modify.install") < 0) {
        msg(MSG_ERR, "  ! sd-boot: error setting up efi-tool environment\n");
        ret = -1;
        goto exit;
    }

    /*
     * allocate mem for:
     * <var><entry_one><entry_two>
     */
    size_t len = 0;
    len = strlen(var) + strlen(entry_one) + strlen(entry_two) + 1;

    env_var = (char *)calloc(len, sizeof(char) + 1);
    if (env_var == nullptr) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    if (snprintf(env_var, len * sizeof(char), "%s%s%s", var, entry_one, entry_two) < 0) {
        msg(MSG_ERR, "  ! sd-boot: error setting up efi-tool environment\n");
        ret = -1;
        goto exit;
    }

    ret = array_str_new(2, env);
    if (ret != 0) {
        goto exit;
    }

    env->rows[0] = env_var;
    env_var = nullptr;
    env->rows[1] = nullptr;

exit:
    if (env_var != nullptr) {
        free((void *)env_var);
    }
    return ret;
}

