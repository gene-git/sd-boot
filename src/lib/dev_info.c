// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Development Set Up.
 * - Supports non-root testing in directory (Testing/__root__)
 * - Activated by non-root user and env variable set:
 *   SDB_DEV_TEST
 *
 */
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sd-boot.h"

enum { 
    VERB_LOW = 0, 
    VERB_HIGH = 2,
};

static int dev_root_init(DevInfo *info) {
    /*
     * Set dev root
     * - must be called within ./Testing or above it
     *   othewise ignored. Testing tool so developers can
     *   test without being root or even running tools like 
     *   kernel-install.
     * - Provides env "BOOT_ROOT" so that kernel-install will install to the testing root
     * - Can only be run in Testing dir or it's parent dir
    */
    int ret = 0;
    char cwd_env[PATH_MAX] = {'\0'};
    char top_dir[PATH_MAX] = {'\0'};
    const char *testing = "Testing";
    const char *root = "__root__";
    char *cwd_base = nullptr;

    if (getcwd(cwd_env, PATH_MAX) == nullptr) {
        perror(nullptr);
        ret = -1;
        goto exit;
    };
    strncpy(top_dir, cwd_env, PATH_MAX);

    cwd_base = basename(cwd_env);

    /*
     * test-root:
     *  ~ <cwd>/Testing/__root__ 
     *  ~ <cwd>/__root__   (if cwd ~/Testing)
     */ 
    if (strncmp(cwd_base, testing, PATH_MAX) != 0) {
        if (snprintf(cwd_env, PATH_MAX, "%s/%s", top_dir, testing) < 0) {
            perror(nullptr);
            ret = -1;
            goto exit;
        }
        strncpy(top_dir, cwd_env, PATH_MAX);
    } 

    if (snprintf(info->root, PATH_MAX, "%s/%s", top_dir, root) < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    /*
     * env:
     * - BOOT_ROOT
     */
    ret = array_str_new(2, &info->env_boot_root);
    if (ret != 0) {
        ret = -1;
        goto exit;
    }

    char tmp_path[PATH_MAX] = {'\0'};
    if (snprintf(tmp_path, PATH_MAX, "BOOT_ROOT=%s/boot", info->root) < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    info->env_boot_root.rows[0] = strdup(tmp_path);
    if (info->env_boot_root.rows[0] == nullptr) {
        ret = -1;
        goto exit;
    }
    info->env_boot_root.rows[1] = nullptr;

    /*
     * -KERNEL_INSTALL_PLUGINS
     */
    ret = ki_plugins_test_env(info->root, &info->env_all_plugins);
    if (ret != 0) {
        goto exit;
    }

exit:
    if (ret != 0) {
        array_str_free(&info->env_boot_root);
        array_str_free(&info->env_all_plugins);
    }
    return ret;
}

static void dev_tree_setup(DevInfo *info) {
    /*
     * Make a usable development test treee anchored at info->root
     * - we dont know if test system has efi on /efi or /boot
     *   set up both for simplicity
     * Testing is part of project.
    */ 
    const char *dirs[] = {
        "boot",
        "boot/EFI",
        "boot/EFI/systemd",
        "efi",
        "efi/EFI",
        "efi/EFI/systemd",
        "etc",
        "etc/sd-boot",
        "etc/kernel",
        "etc/kernel/install.d",
        "usr",
        "usr/bin",
        "usr/lib",
        "usr/lib/sd-boot",
        "tmp",
        "var",
        "var/lib",
        "var/lib/sd-boot",
    };

    msg(MSG_VERB, " Devtest Mode: using test root-dir : %s\n", info->root);

    /*
     * make testing root tree
     */
    int num_dirs = sizeof(dirs)/sizeof(dirs[0]);
    char path[PATH_MAX] = {'\0'};

    for (int i = 0; i < num_dirs; i++) {
        if (snprintf(path, PATH_MAX, "%s/%s", info->root, dirs[i]) < 0) {
            perror(nullptr);
            return ;
        };
        if (makedir(path, 0) != 0) {
            msg(MSG_ERR, " Error creating dir %s\n", path);
            return;
        }
    }
}

/*
 * Initialize Developer Test Mode.
 *
 * When not active info->root = "/".
 * When active info->root ~ "Testing/__root__"
 *
 * Env SDB_DEV_TEST => testing mode.,
 * - Work is done inside a separate directory root
 * - In test mode:
 *   * test <test-root> = ./Testing/__root__ 
 *   * <test-root>/etc/sd-boot/
 *     used for inputs
 *   * <test-root> 
 *     /efi, /boot are where kernels are installed.
 * - NB testing tree only applicable for non-root user.
 *
 * Creates 2 environment variables:
 * BOOT_ROOT
 * KERNEL_INSTALL_PLUGINS
 *
 * The latter ensures that the development plugins are used in test mode instead of ones in "/"
 */
int init_devinfo(DevInfo *info) {
    info->test = false;
    info->root[0] = '/';
    info->root[1] = '\0';
    info->euid = geteuid();

    char *test_env = secure_getenv("SDB_DEV_TEST");

    if (test_env != nullptr) {
        info->test = true;
        if (info->euid != 0) {
            dev_root_init(info);
            dev_tree_setup(info);
        } else {
            msg(MSG_ERR, " Error: test mode must be run non-root\n");
            return -1;
        }
    }
    return 0;
}

void clean_devinfo(DevInfo *info) {
    /*
     * Clean up mem - while not strictly needed, can be helpful
     * when looking for memory leaks
     */
    array_str_free(&info->env_boot_root);
    array_str_free(&info->env_all_plugins);
}
