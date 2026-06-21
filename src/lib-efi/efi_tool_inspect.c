// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * "Add" a bootable efi program.
 * Install bootable efi programs (e.g. efi-shell) from the EFI partition.
 * Also handles "inspect" oper.
 */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-msg.h"
#include "sd-boot-package.h"
#include "sd-boot-utils.h"

int efi_tool_inspect(SdBoot *conf, const char *pkg) {
    /*
     * Add one package
     */
    int ret = 0;
    char version[EFI_ROW_MAX] = {};
    char *efi_image = nullptr;
    Array_str env_arr = {};
    Array_str arg_arr = {};

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
    char pkg_version[KV_MAX_VAL_LEN] = {};
    ret = package_version_installed(conf, pkg, KV_MAX_VAL_LEN, pkg_version);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: package %s is not installed\n", pkg);
        ret = 1;
        goto exit;
    }

    /*
     * kernel version = pkg-pkg_verssion
     */
    if (snprintf(version, EFI_ROW_MAX-1, "%s-%s", pkg, pkg_version) < 0) {
        msg(MSG_ERR, "  ! sd-boot: efi tool: error building kernel version\n");
        ret = -1;
        goto exit;
    }

    ret = array_str_new((size_t)3, &arg_arr);
    if (ret != 0) {
        goto exit;
    }

    arg_arr.rows[0] = strdup("inspect");
    arg_arr.rows[1] = strdup(version);
    arg_arr.rows[2] = efi_image;
    efi_image = nullptr;

    if (!arg_arr.rows[0] || !arg_arr.rows[1]) {
        ret = -1;
        goto exit;
    }

    array_str_refresh_row_len(&arg_arr);

    ret = kernel_install_run(conf, &arg_arr, &env_arr);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: error inspecting efi tool\n");
        ret = -1;
        goto exit;
    }

exit:
    array_str_free(&env_arr);
    array_str_free(&arg_arr);

    if (efi_image) {
        free((void *)efi_image);
    }
    return ret;
}

