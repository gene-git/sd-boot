// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Determine if efi packaged is managed by sd-boot
 */
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"

bool is_efi_pkg_sd_boot_managed(Array_str *pkgs_arr, const char *pkg) {
    bool is_managed = false;

    if (!pkg || pkg[0] == '\0') {
        goto exit;
    }

    for (size_t i = 0; i < pkgs_arr->num_rows; i++) {
        if (strcmp(pkg, pkgs_arr->rows[i]) == 0) {
            is_managed = true;
            break;
        }
    }
exit:
    return is_managed;
}

int load_efi_tool_packages(SdBoot *conf, Array_str *arr) {
    char path[PATH_MAX] = {};

    if (snprintf(path, PATH_MAX, "%s%s", conf->root, "etc/sd-boot/efi-tool.packages") < 0) {
        perror(nullptr);
        return -1;
    }

    if (read_file(path, arr) != 0) {
        return 1;
    }
    return 0;
}

