// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Returns a list of packages managed by sd-boot taken from the files:
 *
 *  kernel packages:        /etc/sd-boot/kernel.packages
 *  efi tool packages:      /etc/sd-boot/efi-tool.packages
 *
 * Return list of kernel package names using sd-boot to install
 * e.g.
 *  int num_pkgs;
 *  char **pkgs;
 *  load_kernel_packages(&num, &pkgs)
 */
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>

#include "sd-boot.h"

int load_kernel_packages(SdBoot *conf, Array_str *arr) {
    char path[PATH_MAX] = {'\0'};

    if (snprintf(path, PATH_MAX, "%s/%s", conf->info.root, "etc/sd-boot/kernel.packages") < 0) {
        perror(nullptr);
        return -1;
    }

    if (read_file(path, arr) != 0) {
        return 1;
    }
    return 0;
}

int load_efi_tool_packages(SdBoot *conf, Array_str *arr) {
    char path[PATH_MAX] = {'\0'};

    if (snprintf(path, PATH_MAX, "%s/%s", conf->info.root, "etc/sd-boot/efi-tool.packages") < 0) {
        perror(nullptr);
        return -1;
    }

    if (read_file(path, arr) != 0) {
        return 1;
    }
    return 0;
}
