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
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"


static int efi_tool_add(SdBoot *conf, char *pkg) {
    /*
     * Add one package
     */
    int ret = 0;
    PackageVersion pkg_vers = {};
    char *curr = nullptr;
    char *prev = nullptr;
    char version[ROW_MAX] = {'\0'};
    char *efi_image = nullptr;
    Array_str env_arr = {};

    msg(MSG_NORMAL, "sd-boot: Updating bootable efi tool %s\n", pkg);

    /*
     * set up special efi plugins environ to use
     */
    ret = ki_plugins_efi_update_env(conf->info.root, &env_arr);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    /*
     * update saved package versions
     * - pkg_vers now contains updated curr/prev
     */
    ret = update_package_versions(conf, pkg, &pkg_vers);
    if (ret != 0) {
        goto exit;
    }
    curr = pkg_vers.current;
    prev = pkg_vers.previous;

    if (curr[0] == '\0') {
        msg(MSG_ERR, "sd-boot: add efi tool: no curr version\n");
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

        msg(MSG_NORMAL, "  sd-boot: removing prev %s %s\n", pkg, prev);

        if (snprintf(version, ROW_MAX-1, "%s-%s", pkg, prev) < 0) {
            msg(MSG_ERR, "sd-boot: efi tool: error making prev version\n");
            ret = -1;
            goto exit;
        }
        char *cmd_args[] = {"remove", version, nullptr};

        ret = kernel_install_run(conf, cmd_args, env_arr.rows);
        if (ret != 0) {
            msg(MSG_ERR, "sd-boot: error removing prev vers %s\n", version);
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
        msg(MSG_ERR, "sd-boot: add efi tool: no efi image found\n");
        ret = -1;
        goto exit;
    }

    /*
     * version = <package>-<curr>
     */
    if (snprintf(version, ROW_MAX-1, "%s-%s", pkg, curr) < 0) {
        msg(MSG_ERR, "sd-boot: efi tool: error making curr vers\n");
        ret = -1;
        goto exit;
    }

    msg(MSG_NORMAL, "  sd-boot: adding %s %s\n", pkg, curr);

    char *cmd_args[] = {"add", version, efi_image, nullptr};
    ret = kernel_install_run(conf, cmd_args, env_arr.rows);
    if (ret != 0) {
        msg(MSG_ERR, "sd-boot: error installing efi tool\n");
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

static int efi_tool_remove(SdBoot *conf, char *pkg) {
    /*
     * Remove one package
     */
    PackageVersion pkg_vers = {};
    int ret = 0;
    char version[ROW_MAX] = {'\0'};
    Array_str env_arr = {};

    msg(MSG_NORMAL, "sd-boot: Removing efi tool %s\n", pkg);
    
    ret = ki_plugins_efi_update_env(conf->info.root, &env_arr);
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
            msg(MSG_ERR, "sd-boot: efi tool: remove %s : no version found\n", pkg);
            goto exit;
            break;

        default:
            break;
    }

    /*
     * found version - lets remove it.
     */
    if (snprintf(version, ROW_MAX-1, "%s-%s", pkg, pkg_vers.current) < 0) {
        msg(MSG_ERR, "sd-boot: efi tool: error making curr vers\n");
        ret = -1;
        goto exit;
    }
    
    /*
     * remove install using kernel-install remove
     */
    char *cmd_args[] = {"remove", version, nullptr};
    ret = kernel_install_run(conf, cmd_args, env_arr.rows);
    if (ret != 0) {
        msg(MSG_ERR, "sd-boot: error removing efi tool\n");
        ret = -1;
        goto exit;
    }

    /*
     * remove package version file.
     */
    ret = remove_package_versions(conf, pkg);
    if (ret != 0) {
        msg(MSG_ERR, "sd-boot: error installing efi filesystem drivers\n");
        return 1;
        goto exit;
    }

exit:
    array_str_free(&env_arr);
    return ret;
}

int main(int argc, char *argv[]) {
    /*
     * Install efi tool (like efi-shell or memtest86) into the efi.
     *
     * Takes one argument:
     * - add or remove
     *
     * - add:
     *   Add the bootable efi image using kernel-install into the efi..
     *   Boot loader entry will be created.
     *   If an older entry is present then it is removed
     *
     * - remove
     *   - remove an efi tool from the efi (also removed the loader entry)
     *
     * Triggers:
     *  - package name of the tool.
     *
     * Input Files:
     *  - /etc/sd-boot/<package-name>.image
     *    Contains the path to the efi image file to be installed into boot_root.
     */

    int ret = 0;
    SdBoot conf = {};

    if (argc < 2) {
        msg(MSG_ERR, "sd-boot: missing add or remove\n");
        ret = 1;
        goto exit;
    }

    /*
     * load config
     * - which sets verbosity level
     */
    if (load_config(&conf) != 0) {
        msg(MSG_ERR, "sd-boot: warning - no config file loaded - skipping\n");
    }

    int oper = BAD;
    oper = kernel_install_oper(argv[1]);
    if (oper == BAD) {
        msg(MSG_ERR, "sd-boot: expect add or remove but got %s\n", argv[1]);
        ret = 1;
        goto exit;
    }

    /*
     * Every trigger on stdin is a package name of the efi-tool
     * - read them 
     * - Each trigger is package name which must appear only once
     *   duplictes could create problems
     */
    Array_str trigs_arr = {};
    if (read_triggers(&trigs_arr) != 0) {
        msg(MSG_ERR, "sd-boot efi tool - error reading triggers\n");
        ret = 1;
        goto exit;
    }

    for (size_t i = 0; i < trigs_arr.num_rows; i++) {
        char pkg[MAX_VAL_LEN] = {'\0'} ;
        strncpy(pkg, trigs_arr.rows[i], MAX_VAL_LEN-1);

        switch (oper) {
            case ADD:
                ret = efi_tool_add(&conf, pkg);
                if (ret != 0) {
                    ret = 1;
                    goto exit;
                }
                break;

            case REMOVE:
                ret = efi_tool_remove(&conf, pkg);
                if (ret != 0) {
                    ret = 1;
                    goto exit;
                }
                break;

            default:
                /*
                 * not used
                 */
                break;
            }
        }
exit:
    array_str_free(&trigs_arr);
    clean_config(&conf);
    return ret;
}

