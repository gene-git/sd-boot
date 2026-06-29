// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-efi.h
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
char *package_to_efi_image(SdBoot *conf, const char *pkg);
char *efi_image_to_package(SdBoot *conf, const char *path);

//int efi_tool_add(SdBoot *conf, PkgInfo *info);
//int efi_tool_inspect(SdBoot *conf, PkgInfo *info);
//int efi_tool_remove(SdBoot *conf, PkgInfo *info);
int efi_tool_update_execute(Tool *tool);
int efi_tool_update_one(SdBoot *conf, PkgInfo *info);

char *efi_tool_ki_vers(char *pkg_name, char *pkg_vers);
int efi_tool_pkginfo(SdBoot *conf, PkgInfo *info);

int efi_tool_pkginfos_from_triggers(TriggerInfo *tinfo, Tool *tool);

int ki_make_kernel_conf_bls(SdBoot *conf);
int ki_efi_update_env(SdBoot *conf, Array_str *env);
int ki_plugins_efi_update_env(char *root, Array_str *env) ;

int loaderentry_modify_efi(SdBoot *conf, KIplugin *plugin);


#endif
