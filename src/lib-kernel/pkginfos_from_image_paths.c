// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Create array of pkginfos for each kernel image path.
 * (managed or not)
 */
#include <stddef.h>

#include "sd-boot-utils.h"
#include "sd-boot-kernel.h"
#include "sd-boot-package.h"
#include "sd-boot.h"

int kernel_pkginfos_from_image_paths(Array_str *image_paths, Tool *tool) {
    int ret = 0;
    Array_str *managed_pkgs = &tool->managed_pkgs;
    size_t num_managed = managed_pkgs->num_rows;
    PkgInfo *pkginfo = nullptr;

    if (image_paths->num_rows == 0) {
        return 0;
    }

    ret = tool_alloc(image_paths->num_rows, tool);
    if (ret != 0) {
        goto exit;
    }

    for (size_t i = 0; i < tool->num_pkgs; i++) {
        /*
         * Update:
         * - pkg_name
         * - ki_image
         * - ki_vers
         * - mod_dir
         */
        pkginfo = &tool->pkginfo[i];

        ret = path_add_slash(image_paths->rows[i], &pkginfo->ki_image);
        if (ret != 0) {
            goto exit;
        }

        ret = init_pkginfo_from_kernel_image(pkginfo);
        if (ret != 0) {
            goto exit;
        }

        /*
         * Update:
         * - pkg_vers
         * - vers_curr, vers_prev
         * - managed
         */
        ret = package_version_installed(&tool->conf, pkginfo);
        if (ret != 0) {
            goto exit;
        }

        if (read_package_version_file(&tool->conf, pkginfo) < 0) {
            ret = -1;
            goto exit;
        }

        pkginfo->managed = string_in_list(pkginfo->pkg_name, num_managed, managed_pkgs->rows);
    }

exit:
    return ret;
}

