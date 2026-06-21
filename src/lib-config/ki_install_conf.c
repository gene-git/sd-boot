// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Initilize the directory used for "shadow" install.conf for kernel-install
 *
 * Both the normal and shadow directoroes are relative to conf->root.
 *
 * Initializes:
 *  - kernel_conf_dir
 *  - kernel_conf_bls_dir
 */
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>

#include "sd-boot-config.h"

int ki_install_conf_init(SdBoot *conf) {
    int ret = 0;
    char path[PATH_MAX] = {};

    /*
     * The normal /etc/kernel/install.conf
     */
    if (snprintf(path, sizeof(path), "%s%s", conf->root, "etc/kernel") < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    conf->kernel_conf_dir = strdup(path);


    /*
     * shadow /var/lib/sd-boot/kernel_conf_bls
     * We want kernel_conf_bls_dir at a fixed location.
     * Used by efi tool to make a shadow /etc/kernel with
     * layout set to bls.
     */
    if (snprintf(path, sizeof(path), "%s%s", conf->root, "var/lib/sd-boot/kernel_conf_bls") < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    conf->kernel_conf_bls_dir = strdup(path);

exit:
    return ret;
}
