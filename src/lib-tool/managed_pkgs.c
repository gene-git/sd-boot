// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read list of kernel or efi-tool packages managed by sd-boot
 *
 * Reads file: /etc/sd-boot/<which>.packages
 *
 * <which> = "kernel" or "efi-tool"
 */

#include <linux/limits.h>
#include <stdio.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"


/*
 * Load the list of packages managed by sd-boot into *pkg_list*.
 * If kernel then list is read from:
 *  <root>etc/sd-boot/kernel.packages
 *
 * if efi_tool then list is read from:
 *  <root>etc/sd-boot/efi-tool.packages
 * 
 * If neither kernel nor efi_tool, there is nothing to do.
 * Requesting and getting nothing is not an error.
 */
int read_managed_packages(SdBoot *conf, Array_str *pkg_list) {
    char path[PATH_MAX] = {};
    char *which = nullptr;

    switch(conf->tool_type) {
        case SDB_KERNEL:
            which = "kernel";
            break;

        case SDB_EFI_TOOL:
            which = "efi-tool";
            break;

        default:
            return 0;
            break;
    }

    if (snprintf(path, PATH_MAX, "%setc/sd-boot/%s.packages", conf->root, which) < 0) {
        perror(nullptr);
        return -1;
    }

    if (read_file(path, pkg_list) != 0) {
        return 1;
    }

    return 0;
}


/*
 * Read the list of managed packages into: tool->managed_pkgs
 */
int load_managed_package_list(Tool *tool) {
    int ret = 0;
    ret = read_managed_packages(&tool->conf, &tool->managed_pkgs);
    return ret;
}


