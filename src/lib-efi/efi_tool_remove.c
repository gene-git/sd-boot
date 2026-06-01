// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Update a bootable efi program.
 *
 * Install or remove bootable efi programs (e.g. efi-shell) from the EFI partition.
 *
 * Supports SDB_DEV_TEST which sets BOOT_ROOT to the testing tree.
 */
#include <stdio.h>

#include "sd-boot.h"


int efi_tool_remove(SdBoot *conf, const char *pkg) {
    /*
     * Remove one package
     */
    PackageVersion pkg_vers = {};
    int ret = 0;
    char version[ROW_MAX] = {};
    Array_str env_arr = {};

    msg(MSG_NORMAL, "  ↳ sd-boot: Removing efi tool %s\n", pkg);
    
    ret = ki_efi_update_env(conf, &env_arr);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    /*
     * -1 = error
     *  0 = got package version info
     *  1 = no package version file found
     */
    ret = read_package_versions(conf, pkg, &pkg_vers);
    switch (ret) {
        case -1:
            goto exit;
            break;

        case 1:
            msg(MSG_ERR, "  ! sd-boot: efi tool: remove %s : no version found\n", pkg);
            goto exit;
            break;

        default:
            break;
    }

    /*
     * found version - lets remove it.
     */
    if (snprintf(version, ROW_MAX-1, "%s-%s", pkg, pkg_vers.current) < 0) {
        msg(MSG_ERR, "  ! sd-boot: efi tool: error making curr vers string\n");
        ret = -1;
        goto exit;
    }
    
    /*
     * remove install using kernel-install remove
     */
    char *cmd_args[] = {"remove", version, nullptr};
    ret = kernel_install_run(conf, cmd_args, env_arr.rows);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: error removing efi tool\n");
        ret = -1;
        goto exit;
    }

    /*
     * remove package version file.
     */
    ret = remove_package_versions(conf, pkg);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: error removing package versions file\n");
        return 1;
        goto exit;
    }

exit:
    array_str_free(&env_arr);
    return ret;
}

