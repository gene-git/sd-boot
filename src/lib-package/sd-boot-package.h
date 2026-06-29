// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-package.h
 */
#ifndef SD_BOOT_PACKAGE_H
#define SD_BOOT_PACKAGE_H

#include <stdbool.h>
#include <stddef.h>

#include "sd-boot-config.h"
#include "sd-boot.h"

/*
 * Function declarations
 */
int update_package_versions(SdBoot *conf, PkgInfo *pkginfo);
int package_version_installed(SdBoot *conf, PkgInfo *pkginfo);

int read_package_version_file(SdBoot *conf, PkgInfo *pkginfo);
int write_package_version_file(SdBoot *conf, PkgInfo *pkginfo);
int remove_package_version_file(SdBoot *conf, PkgInfo *pkginfo);

#endif
