// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Wrapper to run kernel-install
 *
 * - In development mode, the outputs go to the testing root (conf->root)
 * - dev mode is active whenev euid is non-root and the env SBD_DEV_TEST is set.
 */
#include <stdlib.h>
#include <string.h>

#include "sd-boot-cmd.h"
#include "sd-boot-config.h"
#include "sd-boot-msg.h"
#include "sd-boot-utils.h"
#include <sd-boot.h>


/*
 * Solely for 90-loaderentry.install in test dev area
 * Set BOOT_MNT same as BOOT_ROOT
 */
static char *env_boot_mnt(SdBoot *conf) {
    char *var = nullptr;
    const char *env_name = "BOOT_MNT";
    size_t len = 0;
    
    if (!conf->env_boot_root.rows) {
        return var;
    }

    /*
     * boot_root includes the '=' sign
     */
    const char *boot_root_env = conf->env_boot_root.rows[0];
    const char *boot_root = strchr(boot_root_env, '=');
    if (!boot_root) {
        return var;
    }

    len = strlen(env_name) + strlen(boot_root) + 1;

    var = calloc(len, sizeof(char));
    if (var) {
        strlcat(var, env_name, len);
        strlcat(var, boot_root, len);
    }
    return var;
}

/*
 * Environ: Combine 
 * - BOOT_ROOT
 * - PATH
 * - envp provided by caller
 */
static int env_init(SdBoot *conf, Array_str *envp, Array_str *env_all) {
    int ret = 0;
    char *env_var = nullptr;

    ret = array_str_copy_rows(&conf->env_base, env_all);
    if (ret != 0) {
        goto exit;
    }

    ret = array_str_copy_rows(&conf->env_boot_root, env_all);
    if (ret != 0) {
        goto exit;
    }

    ret = array_str_copy_rows(envp, env_all);
    if (ret != 0) {
        goto exit;
    }

    /*
     * loader entry helper for test
     * 90-loaderentry.install - gets confused when in test mode only
     * - when in test mode it tries to identify the root of EFI filesystem using
     *   stat -c %m "$KERNEL_INSTALL_BOOT_ROOT"
     * - this leafs to very long path to efi executable that is odd.
     * - fix this by providing BOOT_MNT env variable with the correct boot root
     * Not crucial since test mode only tests the file paths - nothing is 'booted'
     * from this directory.
     */
    if (conf->test && (conf->is_efi_tool || !conf->is_uki)) {
        env_var = env_boot_mnt(conf);
        if (env_var) {
            size_t num = env_all->num_rows;
            ret = array_str_resize(num + 1, env_all);
            if (ret != 0) {
                goto exit;
            }
            env_all->rows[num] = env_var;
            env_var = nullptr;
        }
    }

    /*
     * Wrap it up
     */
    ret = array_str_null_terminate(env_all);
    if (ret != 0) {
        goto exit;
    }

    array_str_refresh_row_len(env_all);

exit:
    if (env_var) {
        free((void *)env_var);
    }
    return ret;
}


/*
 * --esp-path=<path to EFI>
 * --boot-path=<..>
 */
static void esp_boot_path_option_test(SdBoot *conf, char **opt_esp, char **opt_boot) {
    size_t len = 0;
    const char *o_esp = "--esp-path=";
    const char *o_boot = "--boot-path=";
    const char *efi = "boot";
    size_t base_len = 0;

    base_len = strlen(conf->root) + strlen(efi) + 1;

    /*
     * --esp-path=
     */
    len = strlen(o_esp) + base_len;
    *opt_esp = calloc(len, sizeof(char));
    if (*opt_esp == nullptr) {
        return;
    }
    strlcat(*opt_esp, o_esp, len);
    strlcat(*opt_esp, conf->root, len);
    strlcat(*opt_esp, efi, len);

    /*
     * --boot-path=
     */
    len = strlen(o_boot) + base_len;
    *opt_boot = calloc(len, sizeof(char));
    if (*opt_boot == nullptr) {
        return;
    }

    strlcat(*opt_boot, o_boot, len);
    strlcat(*opt_boot, conf->root, len);
    strlcat(*opt_boot, efi, len);

}

/*
 * Argv 
 */
static int arg_init(SdBoot *conf, Array_str *argp, Array_str *arg_all) {

    int ret = 0;

    ret = array_str_new(1, arg_all); 
    if (ret != 0) {
        goto exit;
    }

    arg_all->rows[0] = strdup("/usr/bin/kernel-install");
    if (!arg_all->rows[0]) {
        ret = -1;
        goto exit;
    }

    ret = array_str_copy_rows(argp, arg_all);
    if (ret != 0) {
        goto exit;
    }

    /*
     * When verb >= 3 pass "-v" to kernel-install
     *
     */
    if (conf->verb >= 3) {
        size_t num = arg_all->num_rows;
        ret = array_str_resize(num + 1, arg_all);
        if (ret != 0) {
            goto exit;
        }
        arg_all->rows[num] = strdup("-v");
        if (!arg_all->rows[num]) {
            ret = -1;
            goto exit;
        }
    }

    /*
     * In test / dev mode set ESP path for kernel-install
     */
    bool avoid = true;
    if (conf->test && !avoid) {
        char *opt_esp = nullptr;
        char *opt_boot = nullptr;

        esp_boot_path_option_test(conf, &opt_esp, &opt_boot);


        if (opt_esp && opt_boot) {

            size_t num = arg_all->num_rows;

            ret = array_str_resize(num + 2, arg_all);
            if (ret != 0) {
                goto exit;
            }

            arg_all->rows[num] = opt_esp;
            arg_all->rows[num + 1] = opt_boot;
        }
    }

    ret = array_str_null_terminate(arg_all);
    if (ret != 0) {
        goto exit;
    }

    array_str_refresh_row_len(arg_all);

exit:
    return ret;
}

int kernel_install_run(SdBoot *conf, Array_str *argp, Array_str *envp) {
    /*
     * Runs:
     *   kernel-install add <vers> <image>
     *   kernel-install remove <vers>
     *
     * Args:
     *  args = The arguments to pass to kernel-install.
     *  envp = the environment variables to pass on.
     *
     * Returns 
     *  0 = all well
     */
    int ret = 0;
    Array_str env_all = {};
    Array_str arg_all = {};

    /*
     * Env variables
     */
    ret = env_init(conf, envp, &env_all);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Argv 
     * - cmd, arg1, arg2, ... 
     * - use system installed kernel-install
     */
    ret = arg_init(conf, argp, &arg_all);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Run kernel-install
     */
    int child_ret = 0;

    ret = run_cmd(arg_all.rows, env_all.rows, &child_ret);
    if (ret != 0) {
        msg(MSG_ERR, "  ! Error running %s\n", arg_all.rows[0]);
        ret = -1;
        goto exit;
    }

exit:
    array_str_free(&env_all);
    array_str_free(&arg_all);

    return ret;
}
