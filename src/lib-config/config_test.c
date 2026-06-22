// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Development Test Set Up.
 *
 * Note that the development directory structure is:
 * - src/tests/Testing
 * - when in test mode the current working dir must be src/tests/Testing or src/tests.
 * 
 * When in test mode 
 * - Supports non-root testing in directory (Testing/__root__/)
 * - Activated by non-root user and env variable set:
 *   SDB_DEV_TEST
 *
 */
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sd-boot-config.h"
#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

static int init_test_root_dir(SdBoot *conf) {
    char cwd_env[PATH_MAX] = {};
    char top_dir[PATH_MAX] = {};
    char root[PATH_MAX] = {};
    const char *testing = "Testing";
    char *cwd_base = nullptr;

    if (!getcwd(cwd_env, PATH_MAX)) {
        perror(nullptr);
        return -1;
    };

    if (strlcpy(top_dir, cwd_env, PATH_MAX) >= PATH_MAX) {
        return -1;
    }
    cwd_base = basename(cwd_env);

    /*
     * test-root:
     *  ~ <cwd>/Testing/__root__/
     *  ~ <cwd>/__root__/   (if cwd ~/Testing)
     */ 
    if (strcmp(cwd_base, testing) != 0) {
        if (snprintf(cwd_env, PATH_MAX, "%s/%s", top_dir, testing) < 0) {
            perror(nullptr);
            return -1;
        }
        if (strlcpy(top_dir, cwd_env, PATH_MAX) >= PATH_MAX) {
            return -1;
        }
    } 

    if (snprintf(root, PATH_MAX, "%s/__root__/", top_dir) < 0) {
        perror(nullptr);
        return -1;
    }

    conf->test_root = strdup(root);
    conf->root = strdup(root);
    return 0;
}

/*
 * Initialize the BOOT_ROOT env.
 */
static int init_env_boot_root(SdBoot *conf) {
    int ret = 0;

    ret = array_str_new(1, &conf->env_boot_root);
    if (ret != 0) {
        goto exit;
    }

    char tmp_path[PATH_MAX] = {};
    if (snprintf(tmp_path, PATH_MAX, "BOOT_ROOT=%sboot", conf->root) < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    conf->env_boot_root.rows[0] = strdup(tmp_path);
    if (conf->env_boot_root.rows[0] == nullptr) {
        ret = -1;
        goto exit;
    }
    array_str_refresh_row_len(&conf->env_boot_root);

exit:
    if (ret != 0) {
        array_str_free(&conf->env_boot_root);
    }
    return ret;
}

/*
 * Make a usable development test tree anchored at conf->root
 * - we dont know if test system has efi on /efi or /boot
 *   set up both for simplicity
 */ 
static int init_test_tree(SdBoot *conf) {

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
        "usr",
        "usr/bin",
        "usr/lib",
        "usr/lib/kernel/install.d",
        "usr/lib/sd-boot",
        "tmp",
        "var",
        "var/lib",
    };

    msg(MSG_VERB, "  * Test Mode: root dir : %s\n", conf->test_root);

    /*
     * make testing root tree
     */
    int num_dirs = sizeof(dirs)/sizeof(dirs[0]);
    char path[PATH_MAX] = {};

    for (int i = 0; i < num_dirs; i++) {
        if (snprintf(path, PATH_MAX, "%s%s", conf->test_root, dirs[i]) < 0) {
            perror(nullptr);
            return -1;
        };
        if (makedir(path, 0) != 0) {
            msg(MSG_ERR, "  ! Error creating dir %s\n", path);
            return -1;
        }
    }
    return 0;
}

/*
 * Initialize Developer Test Mode.
 *
 * conf->root ~ "Testing/__root__/"
 */
int config_test_init(SdBoot *conf) {

    if (!conf->test) {
        return 0;
    }

    if (conf->euid == 0) {
        msg(MSG_ERR, "  ! Error: test mode must be a non-root user\n");
        return -1;
    }

    msg(MSG_NORMAL, "  Initializing test mode\n");

    /*
     * test root dir
     */
    if (init_test_root_dir(conf) != 0) {
        return -1;
    }

    /*
     * BOOT_ROOT env
     */
    if (init_env_boot_root(conf) != 0) {
        return -1;
    }

    /*
     * Test Tree
     */
    if (init_test_tree(conf) != 0) {
        return -1;
    }

    return 0;
}

