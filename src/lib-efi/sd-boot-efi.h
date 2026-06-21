// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot.h
 */
#ifndef SD_BOOT_EFI_H
#define SD_BOOT_EFI_H

#include "sd-boot-config.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"

enum {
    EFI_ROW_MAX = 256,
};

/*
 * Function declarations
 */

char *efi_image_to_package(SdBoot *conf, const char *path);
char *package_to_efi_image(SdBoot *conf, const char *pkg);
int efi_tool_add(SdBoot *conf, const char *pkg);
int efi_tool_inspect(SdBoot *conf, const char *pkg);
int efi_tool_remove(SdBoot *conf, const char *pkg);
int ki_make_kernel_conf_bls(SdBoot *conf);
int ki_efi_update_env(SdBoot *conf, Array_str *env);
int loaderentry_modify_efi(SdBoot *conf, KIplugin *plugin);
int ki_plugins_efi_update_env(char *test_root, Array_str *env);
bool is_efi_pkg_sd_boot_managed(Array_str *pkgs_arr, const char *pkg);
int load_efi_tool_packages(SdBoot *conf, Array_str *arr);

#endif
