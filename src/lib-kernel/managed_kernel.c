// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Check if this kernel is managed by sd-boot
 */
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sd-boot.h"

bool is_kernel_sd_boot_managed(Array_str *pkgs_arr, KernelInfo *info) {
    bool good_kernel = false;

    if (info == nullptr || info->package == nullptr || info->package[0] == '\0') {
        goto exit;
    }
    for (size_t i = 0; i < pkgs_arr->num_rows; i++) {
        if (strcmp(info->package, pkgs_arr->rows[i]) == 0) {
            good_kernel = true;
            break;
        }
    }
exit:
    return good_kernel;
}

int load_managed_kernel_packages(SdBoot *conf, Array_str *arr) {
    char path[PATH_MAX] = {};

    if (snprintf(path, PATH_MAX, "%s%s", conf->info.root, "etc/sd-boot/kernel.packages") < 0) {
        perror(nullptr);
        return -1;
    }

    if (read_file(path, arr) != 0) {
        return 1;
    }
    return 0;
}


