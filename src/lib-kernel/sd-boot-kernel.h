// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-kernel.h
 */
#ifndef SD_BOOT_KERNEL_H
#define SD_BOOT_KERNEL_H

#include <stdbool.h>
#include <stddef.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"


/*
 * Function declarations
 */
int init_pkginfo_from_kernel_image(PkgInfo *info);
int init_kernel_pkginfos_all(Tool *tool);
int is_kernel_image_path(char *image, bool *is_kernel);

int kernel_pkginfos_from_triggers(TriggerInfo *tinfo, Tool *tool);
int parse_kernel_triggers(TriggerInfo *tinfo);

int kernel_update_execute(Tool *tool);
int kernel_update_one(SdBoot *conf, PkgInfo *info);

int loaderentry_modify_kernel(SdBoot *conf, KIplugin *plugin);

int kernel_pkginfo_from_kernel_image(Tool *tool, PkgInfo *info);
int kernel_pkginfos_all(Tool *tool);
int kernel_pkginfos_from_image_paths(Array_str *image_paths, Tool *tool);
int kernel_pkg_name_to_image(char *pkg_name, char **image_p);

int get_all_kernel_image_paths(Array_str *image_paths);
int kernel_pkginfo_from_pkg_name(Tool *tool, PkgInfo *info);
int kernel_mod_dir_to_pkg_name(PkgInfo *info);

#endif
