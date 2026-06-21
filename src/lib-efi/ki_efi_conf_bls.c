// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Make a shadow of /etc/kernel in /var/lib/sd-boot/kernel_conf_bls which
 * has symlinks of /etc/kernel/xxx except for install.conf
 *
 * install conf is a copy where layout = bls
 *
 * This is only necessary in the event that the layout is not already bls.
 * but the decision to make the copy is up to the caller.
 */
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-utils.h"

/*
 * Creates the source and destination "install.conf"
 * src ~ conf->kernel_conf_dir/install.conf
 * dst ~ conf->kernel_conf_bls_dir/install.conf
 */
static int get_install_conf_paths(SdBoot *conf, char *src, char *dst) {
    int ret = 0;
    const char *file = "install.conf";

    /*
     * Original install.conf
     */
    if (snprintf(src, PATH_MAX, "%s/%s", conf->kernel_conf_dir, file) < 0) {
        ret = -1;
        goto exit;
    }

    /*
     * BLS layout version
     */
    if (snprintf(dst, PATH_MAX, "%s/%s", conf->kernel_conf_bls_dir, file) < 0) {
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}

int ki_make_kernel_conf_bls(SdBoot *conf) {
    int ret = 0;
    Array_str skip = {};
    char src[PATH_MAX] = {};
    char dst[PATH_MAX] = {};
    KInstallMods kinstall_mods = {};

    ret = array_str_new(1, &skip);
    if (ret != 0) {
        goto exit;
    }

    skip.rows[0] = strdup("install.conf");
    if (!skip.rows[0]) {
        ret = -1;
        goto exit;
    }
    array_str_refresh_row_len(&skip);

    /*
     * Make new kernel conf dir
     */
    ret = dir_dup_links(conf->kernel_conf_dir, conf->kernel_conf_bls_dir, &skip);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Make install.conf with layout=bls
     */
    ret = get_install_conf_paths(conf, src, dst);
    if (ret != 0) {
        goto exit;
    }

    kinstall_mods.layout = "layout = bls\n";
    kinstall_mods.initrd_generator = "initrd_generator = \n";
    kinstall_mods.uki_generator = "uki_generator = \n";

    ret = make_kernel_install_conf(&kinstall_mods, src, dst);
    if (ret != 0) {
        goto exit;
    }

exit:
    array_str_free(&skip);
    return ret;
}
