// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Determine -existing- $BOOT per kernel-install rules for BLS layout
 * This is only used to locate old installs, so only find the already existing $BOOT.
 *
 * The rules require:
 *
 * a) Check each directory in turn:
 *
 *    /efi, /boot, /boot/efi
 *
 * b) and Look for:
 *  loader/entries, <entry-token>
 *
 * First directory found is $BOOT
 *
 * See man kernel-install 
 *
 * Returns allocated string if found else null.
 */
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sd-boot-config.h"
#include "sd-boot.h"

char *find_dollar_boot_bls(SdBoot *conf) {
    const char *dirs[] = {
        "efi",
        "boot",
        "boot/efi",
    };
    enum { NUM_DIRS = 3};

    char *subdirs[] = {
        "loader/entries",
        conf->entry_token,
    };
    enum { NUM_SUBDIRS = 2};

    InstallConf *iconf = &conf->install_conf;

    /*
     * boot_root 
     * - if test => conf->root
     * - else if conf->install_conf.boot_root 
     * - else conf->root
     */
    char *root = nullptr;
    if (conf->test || !iconf->boot_root) {
        root = conf->root;

    } else if (iconf->boot_root) {
        root = iconf->boot_root;
    }

    if (!root) {
        return nullptr;
    }

    /*
     * Find boot root
     */
    int ret = 0;
    char *dollar_boot = nullptr;

    for (size_t i = 0; i < NUM_DIRS; i++) {
        char dboot[PATH_MAX] = {};
        char path[PATH_MAX] = {};
        size_t size_dboot = sizeof(path);
        size_t size_path = sizeof(path);

        ret = snprintf(dboot, size_dboot, "%s%s", root, dirs[i]);
        if (ret < 0 || ret > (int)size_dboot) {
            continue;
        }

        for (size_t j = 0; j < NUM_SUBDIRS; j++) {
            if (!subdirs[j] || subdirs[j][0] == '\0') {
                continue;
            }

            ret = snprintf(path, size_path, "%s/%s", dboot, subdirs[j]);
            if (ret < 0 || ret > (int)size_path) {
                continue;
            }

            if (access(path, F_OK) == 0) {
                dollar_boot = strdup(dboot);
                return dollar_boot;
            }
        }
    }
    return nullptr;
}

