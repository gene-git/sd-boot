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
 * b) ki_plugins_efi_update_env()
 *    When installing an efi-tool set the plugins to use that are relevant.
 *    Which is only the plugins to manage loader entries. The other plugins
 *    are relevant only for kernels not efi tools.
 *
 * This is only used in testing.
 */
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

int ki_plugins_test_env(char *test_root, Array_str *env) {
    /*
     * Env will be all the  *.install files from 
     * - standard plugins from /usr/lib/kernel/install.d
     * - plugins from "<test_root>/etc/kernel/install.d
     */
    int ret = 0;
    char *var = nullptr;
    Array_str standard_plugins = {};
    Array_str testing_plugins = {};

    ret = file_list_glob("/usr/lib/kernel/install.d/*.install", &standard_plugins);
    if (ret != 0) {
        goto exit;
    }

    char pattern[PATH_MAX] = {'\0'};
    if (snprintf(pattern, PATH_MAX, "%s/%s", test_root, "etc/kernel/install.d/*.install") < 0) {
        ret = -1;
        goto exit;
    }

    ret = file_list_glob(pattern, &testing_plugins);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Combine them
     */
    const char *env_name = "KERNEL_INSTALL_PLUGINS=";
    size_t bytes = strlen(env_name) + 1;

    for (size_t i=0; i < standard_plugins.num_rows; i++) {
        bytes += standard_plugins.row_len[i] + 1;
    }
    for (size_t i=0; i < testing_plugins.num_rows; i++) {
        bytes += testing_plugins.row_len[i] + 1;
    }

    var = (char *)calloc(bytes, sizeof(char));
    if (var == nullptr) {
        ret = -1;
        goto exit;
    }

    if (snprintf(var, bytes, "%s", env_name) < 0) {
        ret = -1;
        goto exit;
    }

    for (size_t i=0; i < standard_plugins.num_rows; i++) {
        strlcat(var, standard_plugins.rows[i], bytes);
        strlcat(var, " ", bytes);
    }
    for (size_t i=0; i < testing_plugins.num_rows; i++) {
        strlcat(var, testing_plugins.rows[i], bytes);
        strlcat(var, " ", bytes);
    }

    ret = array_str_new(2, env);
    if (ret != 0) {
        goto exit;
    }
    env->rows[0] = var;
    var = nullptr;
    env->rows[1] = nullptr;

exit:
    array_str_free(&standard_plugins);
    array_str_free(&testing_plugins);
    if (var != nullptr) {
        free((void *)var);
    }

    return ret;
}

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
    const char* var = "KERNEL_INSTALL_PLUGINS=";
    const char *entry_one = "/usr/lib/kernel/install.d/90-loaderentry.install";
    char entry_two[PATH_MAX] = {'\0'};
    char *env_var = nullptr;

    if (env == nullptr) {
        ret = -1;
        goto exit;
    }

    if (snprintf(entry_two, PATH_MAX, " %s/%s", test_root,
                "etc/kernel/install.d/95-sd-boot-loaderentry-modify.install") < 0) {
        msg(MSG_ERR, "sd-boot: error setting up efi-tool environment\n");
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
        msg(MSG_ERR, "sd-boot: error setting up efi-tool environment\n");
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

