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


int efi_tool_add(SdBoot *conf, const char *pkg) {
    /*
     * Add one package
     */
    int ret = 0;
    PackageVersion pkg_vers = {};
    char *curr = nullptr;
    char *prev = nullptr;
    char version[ROW_MAX] = {};
    char *efi_image = nullptr;
    Array_str env_arr = {};

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
    if (*prev != '\0' && strncmp(curr, prev, MAX_VAL_LEN-1) != 0) {

        msg(MSG_NORMAL, "  ↳ sd-boot: removing prev %s %s\n", pkg, prev);

        if (snprintf(version, ROW_MAX-1, "%s-%s", pkg, prev) < 0) {
            msg(MSG_ERR, "  ! sd-boot: efi tool: error making prev version\n");
            ret = -1;
            goto exit;
        }
        char *cmd_args[] = {"remove", version, nullptr};

        ret = kernel_install_run(conf, cmd_args, env_arr.rows);
        if (ret != 0) {
            msg(MSG_ERR, "  ! sd-boot: error removing prev vers %s\n", version);
            ret = -1;
            goto exit;
        }
    }

    /*
     * Install new (current) version
     */

    /*
     * get the efi image path
     */
    efi_image = package_to_efi_image(conf, pkg);
    if (efi_image == nullptr) {
        msg(MSG_ERR, "  ! sd-boot: add efi tool: no efi image found\n");
        ret = -1;
        goto exit;
    }

    /*
     * version = <package>-<curr>
     */
    if (snprintf(version, ROW_MAX-1, "%s-%s", pkg, curr) < 0) {
        msg(MSG_ERR, "  ! sd-boot: efi tool: error making curr vers\n");
        ret = -1;
        goto exit;
    }

    msg(MSG_NORMAL, "  ↳ sd-boot: adding %s %s\n", pkg, curr);

    char *cmd_args[] = {"add", version, efi_image, nullptr};
    ret = kernel_install_run(conf, cmd_args, env_arr.rows);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: error installing efi tool\n");
        ret = -1;
        goto exit;
    }

exit:
    if (efi_image != nullptr) {
        free((void *)efi_image);
    }
    array_str_free(&env_arr);
    return ret;
}

