// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Standalone tool to update kernel.
 * Usage:
 *  sd-boot-update-kernel <oper> <package-name>
 *  - <oper> is one of: add, remove or inspect.
 *
 * The package must be already installed by pacman.
 * This tool installs the program provided by the package into /boot (or /efi).
 *
 * Supports SDB_DEV_TEST which sets BOOT_ROOT to the testing tree.
 * See also man kernel-install.
 */
#include <stdbool.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-kernel.h"
#include "sd-boot-msg.h"
#include "sd-boot-utils.h"


static void usage() {
    msg(MSG_ERR, "! sd-boot-kernel-update: Usage: \n");
    msg(MSG_ERR, "    sd-boot-kernel-update <oper> <package-name>\n");
    msg(MSG_ERR, "              <oper> = add or remove\n");
    msg(MSG_ERR, "      <package-name> = name of package to update\n");
}

struct Work {
    SdBoot conf;
    const char *pkg;
    KernelInfo info;
    Array_str pkgs_arr;
    bool is_sd_boot_managed;
};

static void work_clean(struct Work *work) {
    config_clean(&work->conf);
    kernel_info_free(&work->info);
    array_str_free(&work->pkgs_arr);
}


/**
 * - Check arguments.
 * - load config
 * - read trigger list
 * Input:
 *      argc, argv
 * Returns:
 *      oper  = add or remove
 *      conf  = config file data
 *      trigs = Triggers data read from stdin
 */
static int initialize(int argc, char *argv[], struct Work *work) {
    /*
     * Command line & Config
     */
    int ret = 0;
    const char *self = "kernel-update";

    if (argc < 3) {
        usage();
        ret = 1;
        goto exit;
    }
    const char *oper_str = argv[1];
    const char *pkg = argv[2];

    work->conf.oper = KI_BAD;
    work->conf.oper = kernel_install_oper(oper_str);
    work->conf.oper_str = strdup(oper_str);

    if (work->conf.oper == KI_BAD) {
        msg(MSG_ERR, "! sd-boot: %s expect add, remove or inspect but got %s\n", self, oper_str);
        ret = 1;
        goto exit;
    }

    if (load_config(&work->conf) != 0) {
        msg(MSG_VERB, "- sd-boot: warning failed to load config config file\non");
    }

    if (!check_permission(&work->conf)) {
        ret = 1;
        goto exit;
    }

    /*
     * Get info about this package
     */
    ret = kernel_pkg_to_info(pkg, &work->info);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    /*
     * kernels managed by sd-boot
     */
    ret = load_managed_kernel_packages(&work->conf, &work->pkgs_arr);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    work->is_sd_boot_managed = is_kernel_sd_boot_managed(&work->pkgs_arr, &work->info);

exit:
    return ret;
}

/**
 * Takes one argument:
 * - add or remove
 *
 * - add:
 *   - Install or update kernel into boot_root
 *
 * - remove
 *   - remove kernel files from boot_root
 * Triggers:
 *      a) /usr/lib/modules/<kenr-vers>/vmlinux
 *      b) non-kernel paths
 *      c) any packages which impact initrd or uki image.
 *
 * Required Files:
 *  a) /etc/sd-boot/kernel-install.packages
 *     List of kernel packages managed by sd-boot
 *  b) /usr/lib/modules/<kern-vers>/<pkgbase>
 *
 *    This file provides the package name of that kernel.
 *    <pkgbase> file will be looked for in order:
 *      gc-pkgbase pkgbase-sdb or pkgbase
 */
int main(int argc, char *argv[]) {
    int ret = 0;
    struct Work work = {};

    ret = initialize(argc, argv, &work);
    if (ret != 0) {
        goto exit;
    }

    if (!work.is_sd_boot_managed) {
        goto exit;
    }

    switch (work.conf.oper) {
        case KI_ADD:
        case KI_REMOVE:
            ret = kernel_add_remove(&work.conf, &work.pkgs_arr, &work.info);
            break;

        case KI_INSPECT:
            ret = kernel_inspect(&work.conf, &work.pkgs_arr, &work.info);
            break;

        default:
            msg(MSG_ERR, "  ! sd-boot: unsupported oper: %s\n", work.conf.oper_str);
            break;
    }


exit:
    work_clean(&work);
    return ret;
}

