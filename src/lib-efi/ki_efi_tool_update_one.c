// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * "Add" a bootable efi program.
 * Install bootable efi programs (e.g. efi-shell) from the EFI partition.
 * Also handles "inspect" oper.
 */
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-msg.h"
#include "sd-boot-package.h"
#include "sd-boot-utils.h"

static int remove_prev_vers(SdBoot *conf, PkgInfo *info, Array_str *env_arr) {

    int ret = 0;
    char *ki_vers = nullptr;
    Array_str arg_arr = {};

    if (!info->vers_prev || strcmp(info->vers_prev, info->vers_curr) == 0) {
        return 0;
    }

    msg(MSG_NORMAL, "  ↳ sd-boot: removing prev %s %s\n", info->pkg_name, info->vers_prev);

    ki_vers = efi_tool_ki_vers(info->pkg_name, info->vers_prev);
    if (!ki_vers) {
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
    arg_arr.rows[1] = ki_vers;
    ki_vers = nullptr;

    if (!arg_arr.rows[0] || !arg_arr.rows[1]) {
        ret = -1;
        goto exit;
    }

    array_str_refresh_row_len(&arg_arr);

    ret = kernel_install_run(conf, &arg_arr, env_arr);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: error removing prev vers %s\n", arg_arr.rows[1]);
        ret = -1;
        goto exit;
    }
exit:
    array_str_free(&arg_arr);
    if (!ki_vers) {
        free((void *)ki_vers);
    }
    return ret;
}

/*
 * Prep args and env.
 * For add we use env_plugins already initialized.
 * Not used by 'remove previous'
 * is used by oper = add, remove or inspect.
 */
static int init_arg_arr(SdBoot *conf, PkgInfo *info, Array_str *arg_arr) {

    int ret = 0;

    switch (conf->oper) {
        case KI_REMOVE:
            ret = array_str_new(2, arg_arr);
            if (ret != 0) {
                goto exit;
            }

            arg_arr->rows[0] = strdup(conf->oper_str);
            arg_arr->rows[1] = strdup(info->ki_vers);       // same as vers_curr

            if (!arg_arr->rows[0] || !arg_arr->rows[1]) {
                ret = -1;
                goto exit;
            }
            break;

        case KI_ADD:
        case KI_INSPECT:
            ret = array_str_new(3, arg_arr);
            if (ret != 0) {
                goto exit;
            }

            arg_arr->rows[0] = strdup(conf->oper_str);
            arg_arr->rows[1] = strdup(info->ki_vers);     // info->vers_curr for addremove
            arg_arr->rows[2] = strdup(info->ki_image);

            if (!arg_arr->rows[0] || !arg_arr->rows[1] || !arg_arr->rows[2]) {
                ret = -1;
                goto exit;
            }
            break;

        default:
            goto exit;
            break;
    }

    array_str_refresh_row_len(arg_arr);

exit:
    return ret;
}


int efi_tool_update_one(SdBoot *conf, PkgInfo *info) {
    /*
     * Update one efi-tool
     * - oper ~ add, inspect or remove
     */
    int ret = 0;
    Array_str env_arr = {};
    Array_str arg_arr = {};


    if (!info) {
        msg(MSG_ERR, "  ! sd-boot: efi tool update bad input\n");
        return -1;
    }

    if (!info->managed) {
        msg(MSG_NORMAL, "  not managed by sd-boot - skipping %s\n", info->pkg_name);
        goto exit;
    }

    if (conf->oper == KI_ADD) {
        ret = update_package_versions(conf, info);
        if (ret != 0) {
            goto exit;
        }
    }

    msg(MSG_NORMAL, "⦁ sd-boot: Updating efi tool %s\n", info->pkg_name);

    /*
     * Use special efi plugins via env variable 
     */
    ret = ki_efi_update_env(conf, &env_arr);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    if (conf->oper == KI_ADD) {
        ret = remove_prev_vers(conf, info, &env_arr);
        if (ret != 0) {
            goto exit;
        }
    }

    msg(MSG_NORMAL, "  ↳ sd-boot: %s %s %s\n", conf->oper_str, info->pkg_name, info->pkg_vers);

    /*
     * version = <package-name>-<current-package-version>
     */
    info->ki_vers = efi_tool_ki_vers(info->pkg_name, info->pkg_vers);
    if (!info->ki_vers) {
        msg(MSG_ERR, "  ! sd-boot: efi tool: error making curr vers\n");
        ret = -1;
        goto exit;
    }

    ret = init_arg_arr(conf, info, &arg_arr);
    if (ret != 0) {
        goto exit;
    }

    ret = kernel_install_run(conf, &arg_arr, &env_arr);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: error %s efi tool\n", conf->oper_str);
        ret = -1;
        goto exit;
    }

    /*
     * When removing a package we remove the package version file
     */
    if (conf->oper == KI_REMOVE) {
        ret = remove_package_version_file(conf, info);
        if (ret != 0) {
            msg(MSG_ERR, "  ! sd-boot: error removing package versions file\n");
            goto exit;
        }
    }

exit:
    array_str_free(&arg_arr);
    array_str_free(&env_arr);
    return ret;
}

