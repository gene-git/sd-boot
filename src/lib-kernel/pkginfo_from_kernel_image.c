// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * From kernel image path crate it's the pkginfo
 * Input:
 *  pkginfo->ki_image:
 *
 * Output:
 * - pkginfo->pkg_name
 * - pkginfo->pkg_vers
 *
 * - pkginfo->mod_dir
 * - 
 * - pkginfo->vers_curr
 * - pkginfo->vers_prev
 * - pkginfo->managed
 *
 */

#include <stddef.h>

#include "sd-boot-utils.h"
#include "sd-boot-kernel.h"
#include "sd-boot-package.h"
#include "sd-boot.h"

int kernel_pkginfo_from_kernel_image(Tool *tool, PkgInfo *pkginfo) {
    int ret = 0;

    /*
     * Initialize:
     * info->pkg_name
     * info->mod_dir
     * info->ki_vers
     */
    ret = init_pkginfo_from_kernel_image(pkginfo);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Update:
     * - pkg_vers (if not set)
     * - vers_curr
     * - vers_prev
     * - managed
     */

    if (!pkginfo->pkg_vers) {
        ret = package_version_installed(&tool->conf, pkginfo);
        if (ret != 0) {
            goto exit;
        }
    }

    if (read_package_version_file(&tool->conf, pkginfo) < 0) {
        ret = -1;
        goto exit;
    }

    Array_str *managed_pkgs = &tool->managed_pkgs;
    size_t num_managed = managed_pkgs->num_rows;
    pkginfo->managed = string_in_list(pkginfo->pkg_name, num_managed, managed_pkgs->rows);

exit:
    return ret;
}

