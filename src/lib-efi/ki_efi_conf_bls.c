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
#include <fcntl.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-utils.h"
//#include "sd-boot.h"

/*
 * Creates the source and destination "install.conf"
 * src ~ conf->kernel_conf_dir/install.conf
 * dst ~ conf->kernel_conf_bls_dir/install.conf
 */
static int get_install_conf_path(SdBoot *conf, char *dst) {
    int ret = 0;
    const char *file = "install.conf";

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
     * Make a new simple install.conf layout=bls
     * Other fields are not applicable to efi tools since
     * they are not kernels.
     */
    char install_conf_path[PATH_MAX] = {};
    ret = get_install_conf_path(conf, install_conf_path);
    if (ret != 0) {
        goto exit;
    }

    const char *install_conf = 
        "layout = bls\n"
        "initrd_generator = \n"
        "uki_generator = \n";

    size_t size = strlen(install_conf);
    int flags = WRITE_FLAGS_DEF;
    mode_t mode = WRITE_MODE_DEF;

    ret = write_file(install_conf, size, flags, mode, (const char *)install_conf_path);
    if (ret != 0) {
        goto exit;
    }

exit:
    array_str_free(&skip);
    return ret;
}
