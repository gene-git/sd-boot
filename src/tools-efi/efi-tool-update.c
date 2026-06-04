// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Standalone tool to update a bootable efi program.
 * Usage:
 *  sd-boot-update-efi-tool <oper> <packge name> 
 *  - <oper> is one of: add, remove or inspect.
 *
 * The package must be already installed by pacman in the usual way. 
 * This tool installs the program provided by the package into /boot (or /efi).
 *
 * Supports SDB_DEV_TEST which sets BOOT_ROOT to the testing tree.
 */
#include <stdbool.h>
#include <string.h>

#include "sd-boot.h"

static void usage() {
    msg(MSG_ERR, "! sd-boot-efi-tool-update: Usage:\n");
    msg(MSG_ERR, "    sd-boot-efi-tool-update <oper> <package-name>\n");
    msg(MSG_ERR, "              <oper> = add or remove\n");
    msg(MSG_ERR, "      <package-name> = name of package to update\n");
}

struct Work {
    SdBoot conf;
    const char *pkg;
    char pkg_vers[MAX_VAL_LEN];
    bool is_sd_boot_managed;
};

static void work_clean(struct Work *work) {
    clean_config(&work->conf);
}

static int initialize(int argc, char *argv[], struct Work *work) {
    int ret = 0;
    Array_str pkgs_arr = {};

    if (argc < 3) {
        usage();
        ret = 1;
        goto exit;
    }
    const char *oper_str = argv[1];
    work->pkg = argv[2];

    /*
     * Check package is installed
     */
    ret = package_version_installed(work->pkg, MAX_VAL_LEN, work->pkg_vers);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    /*
     * load config
     * - also sets verbosity level
     */
    if (load_config(&work->conf) != 0) {
        msg(MSG_ERR, "- sd-boot: warning failed to load config file\n");
    }

    if (!check_permission(&work->conf)) {
        ret = 1;
        goto exit;
    }

    work->conf.oper = KI_BAD;
    work->conf.oper_str = strdup(oper_str);
    if (!work->conf.oper_str) {
        msg(MSG_ERR, "! sd-boot: mem allocation error %s\n", oper_str);
        ret = 1;
        goto exit;
    }

    work->conf.oper = kernel_install_oper((const char *)work->conf.oper_str);
    if (work->conf.oper == KI_BAD) {
        msg(MSG_ERR, "! sd-boot: expect add or remove but got %s\n", work->conf.oper_str);
        ret = 1;
        goto exit;
    }

    /*
     * Get list of efi tools managed by sd-boot (if any)
     */
    ret = load_efi_tool_packages(&work->conf, &pkgs_arr);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    work->is_sd_boot_managed = is_efi_pkg_sd_boot_managed(&pkgs_arr, work->pkg);

exit:
    array_str_free(&pkgs_arr);
    return ret;
}

int main(int argc, char *argv[]) {
    /*
     * Install efi tool (like efi-shell or memtest86) into the efi.
     *
     * Takes 2 argument:
     * - oper = add or remove or inspect
     * - package-name
     *
     * - add:
     *   Add the bootable efi image using kernel-install into the efi..
     *   Boot loader entry will be created fo bls layout.
     *   If an older entry is present then it is removed
     * 
     * - inspect
     *   Shows install information about the package 
     *
     * - remove
     *   - remove an efi tool from the efi (and remove any loader entry (bls layout))
     *
     * Required data Files:
     *  - /etc/sd-boot/<package-name>.image
     *    Contains the path to the efi image file to be installed into boot_root.
     */
    int ret = 0;
    struct Work work = {};

    ret = initialize(argc, argv, &work);
    if (ret != 0) {
        goto exit;
    }

    if (!work.is_sd_boot_managed) {
        goto exit;
    }

    /*
     * Do it
     */
    switch (work.conf.oper) {
        case KI_ADD:
            ret = efi_tool_add(&work.conf, work.pkg);
            break;

        case KI_INSPECT:
            ret = efi_tool_inspect(&work.conf, work.pkg);
            break;

        case KI_REMOVE:
            ret = efi_tool_remove(&work.conf, work.pkg);
            break;

        default:
            msg(MSG_ERR, "  ! sd-boot: unsupported oper: %s\n", work.conf.oper_str);
            break;
    }

    if (ret != 0) {
        ret = 1;
        goto exit;
    }

exit:
    work_clean(&work);
    return ret;
}

