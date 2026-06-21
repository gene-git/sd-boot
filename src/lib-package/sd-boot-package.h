// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot.h
 */
#ifndef SD_BOOT_PACKAGE_H
#define SD_BOOT_PACKAGE_H

#include <stdbool.h>
#include <stddef.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"

/*
 * Package Version
 */
typedef struct {
    char pkg[KV_MAX_VAL_LEN+1];
    char current[KV_MAX_VAL_LEN+1];
    char previous[KV_MAX_VAL_LEN+1];
} PackageVersion;

/*
 * Function declarations
 */
int read_package_versions(SdBoot *conf, const char *pkg, PackageVersion *pkg_vers);
int update_package_versions(SdBoot *conf, const char *pkg, PackageVersion *pkg_vers);
int remove_package_versions(SdBoot *conf, const char *pkg);
int package_version_installed(SdBoot *conf, const char *pkg, size_t len_vers, char *vers);

#endif
