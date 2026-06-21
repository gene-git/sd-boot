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
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-msg.h"
#include "sd-boot-package.h"
#include "sd-boot-utils.h"


int efi_tool_remove(SdBoot *conf, const char *pkg) {
    /*
     * Remove one package
     */
    PackageVersion pkg_vers = {};
    int ret = 0;
    char version[EFI_ROW_MAX] = {};
    Array_str env_arr = {};
    Array_str arg_arr = {};

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
    if (snprintf(version, EFI_ROW_MAX-1, "%s-%s", pkg, pkg_vers.current) < 0) {
        msg(MSG_ERR, "  ! sd-boot: efi tool: error making curr vers string\n");
        ret = -1;
        goto exit;
    }
    
    /*
     * remove installed version:  
     * - kernel-install remove version
     */
    ret = array_str_new(2, &arg_arr);
    if (ret != 0) {
        goto exit;
    }

    arg_arr.rows[0] = strdup("remove");
    arg_arr.rows[1] = strdup(version);

    if (!arg_arr.rows[0] || !arg_arr.rows[1]) {
        ret = -1;
        goto exit;
    }

    array_str_refresh_row_len(&arg_arr);

    ret = kernel_install_run(conf, &arg_arr, &env_arr);
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
    array_str_free(&arg_arr);
    array_str_free(&env_arr);
    return ret;
}

