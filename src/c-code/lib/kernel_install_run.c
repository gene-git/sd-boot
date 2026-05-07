// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Wrapper to call kernel-install
 *
 * - Checks for development mode where it uses "fake" kernel install
 *   to allow for code path to be tested.
 * - dev mode is active whenever euid is non-root
 */
#include <stdio.h>
#include <stdlib.h>

#include <sd-boot.h>

int kernel_install_run(SdBoot *conf, char *const args[], char *const envp[]) {
    /*
     * Runs:
     *   kernel-install add <vers> <image>
     *   kernel-install remove <vers>
     *
     * Args:
     *  args = The arguments to pass to kernel-install - null terminated list
     *  envp = the environment variables to pass on - null terminated
     * In dev mode the program execed is kernel-install-test
     * which merely displays it's arguments. We want to still
     * run an external program for better test coverage.
     *
     * Returns 
     * 0 = all well
     * otherise some problem occurred.
     */
    int ret = 0;
    char **envp_full = nullptr;
    char **argv = nullptr;

    /*
     * combine input environ with develop
     * - BOOT_ROOT env for testing.
     */
    size_t num_boot_root = 0;
    if (conf->info.env_boot_root.rows) {
        num_boot_root = count_envp_argv(conf->info.env_boot_root.rows);
    }
    size_t num = 0;
    if (envp != nullptr) {
        num = count_envp_argv(envp);
    }

    size_t num_envp_full = num_boot_root + num + 1;
    envp_full = (char **)calloc(num_envp_full, sizeof(char *));
    if (envp_full == nullptr) {
        ret = -1;
        goto exit;
    }
    for (size_t i = 0; i < num_boot_root; i++) {
        envp_full[i] = conf->info.env_boot_root.rows[i];
    }
    for (size_t i = 0; i < num; i++) {
        envp_full[i + num_boot_root] = envp[i];
    }
    envp_full[num_envp_full - 1] = nullptr;

    /*
     * Build argv to run
     * - leave room for cmd and null
     */
    int num_args = count_envp_argv(args);

    argv = (char **) calloc(num_args + 2, sizeof(char *));
    if (argv == nullptr) {
        msg(MSG_ERR, "sd-boot: Error allocating memory for kenrel-install\n");
        ret = -1;
        goto exit;
    }

    /*
     * use system kernel-install
     */
    char *cmd = "/usr/bin/kernel-install";

    int cnt = 0;
    argv[0] = cmd;
    for (cnt = 0; cnt < num_args; cnt++) {
        argv[cnt+1] = args[cnt];
    }
    argv[cnt+1] = nullptr;

    /*
     * Run kernel-install
     */
    int child_ret = 0;
    ret = run_cmd(argv, envp_full, &child_ret);
    if (ret != 0) {
        msg(MSG_ERR, "Error running %s\n", cmd);
        ret = -1;
        goto exit;
    }

exit:
    if (envp_full != nullptr) {
        free((void *)envp_full);
    }
    if (argv != nullptr) {
        free((void *)argv);
    }
    return ret;
}
