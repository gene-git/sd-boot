// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * "Add" a bootable efi program.
 * Install bootable efi programs (e.g. efi-shell) from the EFI partition.
 * Also handles "inspect" oper.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-msg.h"
#include "sd-boot-package.h"
#include "sd-boot-utils.h"

static int remove_prev_vers(SdBoot *conf, const char *pkg, char *prev, char *curr, Array_str *env_arr) {

    int ret = 0;
    char version[EFI_ROW_MAX] = {};
    Array_str arg_arr = {};

    if (*prev == '\0' || strcmp(curr, prev) == 0) {
        return 0;
    }

    msg(MSG_NORMAL, "  ↳ sd-boot: removing prev %s %s\n", pkg, prev);

    if (snprintf(version, EFI_ROW_MAX-1, "%s-%s", pkg, prev) < 0) {
        msg(MSG_ERR, "  ! sd-boot: efi tool: error making prev version\n");
        ret = -1;
        goto exit;
    }

    /*
     * Build argv
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

    ret = kernel_install_run(conf, &arg_arr, env_arr);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: error removing prev vers %s\n", version);
        ret = -1;
        goto exit;
    }
exit:
    array_str_free(&arg_arr);
    return ret;
}


int efi_tool_add(SdBoot *conf, const char *pkg) {
    /*
     * Add one package
     */
    int ret = 0;
    PackageVersion pkg_vers = {};
    char *curr = nullptr;
    char *prev = nullptr;
    char version[EFI_ROW_MAX] = {};
    char *efi_image = nullptr;
    Array_str env_arr = {};
    Array_str arg_arr = {};

    msg(MSG_NORMAL, "⦁ sd-boot: Updating bootable efi tool %s\n", pkg);

    /*
     * set up special efi plugins environ to use
     */
    ret = ki_efi_update_env(conf, &env_arr);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    /*
     * Add:
     * - update saved package versions
     * - pkg_vers now contains updated curr/prev
     */
    ret = update_package_versions(conf, pkg, &pkg_vers);
    if (ret != 0) {
        goto exit;
    }
    curr = pkg_vers.current;
    prev = pkg_vers.previous;

    if (curr[0] == '\0') {
        msg(MSG_ERR, "  ! sd-boot: add efi tool: no curr version\n");
        ret = -1;
        goto exit;
    }

    /*
     * remove prev version 
     * - skip on update where prev = curr
     * - version string is = "<package>-<prev>"
     *   i.e. augment version string with package name
     */
    ret = remove_prev_vers(conf, pkg, prev, curr, &env_arr);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Install new (current) version
     * - need efi image path
     */
    efi_image = package_to_efi_image(conf, pkg);
    if (!efi_image) {
        msg(MSG_ERR, "  ! sd-boot: add efi tool: no efi image found\n");
        ret = -1;
        goto exit;
    }

    /*
     * version = <package>-<curr>
     */
    if (snprintf(version, EFI_ROW_MAX-1, "%s-%s", pkg, curr) < 0) {
        msg(MSG_ERR, "  ! sd-boot: efi tool: error making curr vers\n");
        ret = -1;
        goto exit;
    }

    msg(MSG_NORMAL, "  ↳ sd-boot: adding %s %s\n", pkg, curr);

    array_str_free(&arg_arr);
    ret = array_str_new(3, &arg_arr); 
    if (ret != 0) {
        goto exit;
    }

    arg_arr.rows[0] = strdup("add");
    arg_arr.rows[1] = strdup(version);
    arg_arr.rows[2] = strdup(efi_image);

    if (!arg_arr.rows[0] || !arg_arr.rows[1] || !arg_arr.rows[2]) {
        ret = -1;
        goto exit;
    }

    array_str_refresh_row_len(&arg_arr);

    ret = kernel_install_run(conf, &arg_arr, &env_arr);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: error installing efi tool\n");
        ret = -1;
        goto exit;
    }

exit:
    if (efi_image) {
        free((void *)efi_image);
    }
    array_str_free(&arg_arr);
    array_str_free(&env_arr);
    return ret;
}

