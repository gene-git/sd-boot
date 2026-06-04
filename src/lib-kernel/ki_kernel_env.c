// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Set up env for kernels with kernel-install (add, remove or inspect)
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

int ki_kernel_update_env(SdBoot *conf, Array_str *env) {
    int ret = 0;
    char **envp_plugins = conf->info.env_all_plugins.rows;
    size_t num_envp_plugins = 0;
    size_t num_envp = 0;

    if (envp_plugins) {
        num_envp_plugins = count_envp_argv(envp_plugins);
    }

    num_envp = num_envp_plugins + 2;
    ret = array_str_new(num_envp, env);
    if (ret != 0) {
        msg(MSG_ERR, "  ! mem alloction error\n");
        goto exit;
    }
    for (size_t i = 0; i < num_envp_plugins; i++) {
        env->rows[i] = strdup(envp_plugins[i]);
        if (env->rows[i] == nullptr) {
            msg(MSG_ERR, "  ! mem alloction error\n");
            ret = -1;
            goto exit;
        }
    }

    const char *path = "PATH=/usr/bin";
    env->rows[num_envp_plugins] = strdup(path);
    if (env->rows[num_envp_plugins] == nullptr) {
        msg(MSG_ERR, "  ! mem alloction error\n");
        ret = -1;
        goto exit;
    }
    env->rows[num_envp_plugins + 1] = nullptr;

exit:
    return ret;
}

