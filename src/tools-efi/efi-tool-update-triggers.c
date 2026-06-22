// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Install or remove a bootable efi program into $BOOT.
 * - called by pacman alpm hook.
 * - Reads stdin to for the list of "triggers" where a trigger is the package name to update.
 *
 * Usage:
 *  sd-boot-efi-tool-update-triggers <oper>
 *  - <oper> is add or remove.
 *
 * The package name is read from stdin.
 *
 * Example of an efi tool package is edk2-shell.
 * Supports SDB_DEV_TEST which sets BOOT_ROOT to the testing tree.
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

struct Work {
    SdBoot conf;
    const char *pkg;
    char pkg_vers[PKG_LEN];
    bool is_sd_boot_managed;

    Array_str trigs_arr;
    Array_str pkgs_arr;
};

static void work_clean(struct Work *work) {
    array_str_free(&work->trigs_arr);
    array_str_free(&work->pkgs_arr);
    config_clean(&work->conf);
}

static int initialize(int argc, char *argv[], struct Work *work) {
    int ret = 0;

    if (argc < 2) {
        msg(MSG_ERR, "! sd-boot: missing add or remove\n");
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
    work->conf.is_efi_tool = true;

    const char *oper_str = argv[1];

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
     * Every trigger on stdin is a package name of the efi-tool
     * - read them 
     * - Each trigger is package name which must appear only once
     *   duplictes could create problems
     */
    if (read_triggers(&work->trigs_arr) != 0) {
        msg(MSG_ERR, "! sd-boot efi tool - error reading triggers\n");
        ret = 1;
        goto exit;
    }

    /*
     * Get efi tools managed by sd-boot (if any)
     */
    ret = load_efi_tool_packages(&work->conf, &work->pkgs_arr);
    if (ret != 0) {
        ret = 0;
        goto exit;
    }

exit:
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
    struct Work work = {};

    ret = initialize(argc, argv, &work);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Triggers are efi tool package name
     */
    for (size_t i = 0; i < work.trigs_arr.num_rows; i++) {
        char pkg[PKG_LEN] = {};

        if (strlcpy(pkg, work.trigs_arr.rows[i], PKG_LEN) >= PKG_LEN) {
            ret = 1;
            goto exit;
        }

        if (!is_efi_pkg_sd_boot_managed(&work.pkgs_arr, pkg)) {
            continue;
        }

        switch (work.conf.oper) {
            case KI_ADD:
                ret = efi_tool_add(&work.conf, pkg);
                if (ret != 0) {
                    ret = 1;
                    goto exit;
                }
                break;

            case KI_REMOVE:
                ret = efi_tool_remove(&work.conf, pkg);
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
    work_clean(&work);
    return ret;
}

