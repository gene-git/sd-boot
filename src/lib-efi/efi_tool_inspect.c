// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * "Add" a bootable efi program.
 * Install bootable efi programs (e.g. efi-shell) from the EFI partition.
 * Also handles "inspect" oper.
 */
#include <stdio.h>
#include <stdlib.h>

#include "sd-boot.h"

int efi_tool_inspect(SdBoot *conf, const char *pkg) {
    /*
     * Add one package
     */
    int ret = 0;
    char version[ROW_MAX] = {};
    char *efi_image = nullptr;
    Array_str env_arr = {};

    msg(MSG_NORMAL, "⦁ sd-boot: inspect bootable efi tool %s\n", pkg);

    /*
     * set up special efi plugins environ to use
     * Note: plugins are not called but we still set up
     * other env vars e.g. shadow config for bls layout.
     */
    ret = ki_efi_update_env(conf, &env_arr);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    /*
     * efi image path
     */
    efi_image = package_to_efi_image(conf, pkg);
    if (!efi_image) {
        msg(MSG_ERR, "  ! sd-boot: inspect efi tool: no efi image found\n");
        ret = -1;
        goto exit;
    }

    /*
     * installed package version 
     */
    char pkg_version[MAX_VAL_LEN] = {};
    ret = package_version_installed(pkg, MAX_VAL_LEN, pkg_version);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: package %s is not installed\n", pkg);
        ret = 1;
        goto exit;
    }

    /*
     * kernel version = pkg-pkg_verssion
     */
    if (snprintf(version, ROW_MAX-1, "%s-%s", pkg, pkg_version) < 0) {
        msg(MSG_ERR, "  ! sd-boot: efi tool: error building kernel version\n");
        ret = -1;
        goto exit;
    }

    char *cmd_args[] = {"inspect", version, efi_image, nullptr};
    ret = kernel_install_run(conf, cmd_args, env_arr.rows);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: error inspecting efi tool\n");
        ret = -1;
        goto exit;
    }

exit:
    array_str_free(&env_arr);
    if (efi_image) {
        free((void *)efi_image);
    }
    return ret;
}

