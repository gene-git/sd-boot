// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Create array of pkinfos for all installed kernels
 * (whether managed or not)
 */
#include <stddef.h>

#include "sd-boot.h"
#include "sd-boot-kernel.h"
#include "sd-boot-package.h"
#include "sd-boot-utils.h"

int kernel_pkginfos_all(Tool *tool) {
    int ret = 0;
    Array_str *managed_pkgs = &tool->managed_pkgs;
    size_t num_managed = managed_pkgs->num_rows;
    PkgInfo *pkginfo = nullptr;

    /*
     * Get:
     * - pkg_name
     * - ki_image
     * - ki_vers
     * - mod_dir
     * Looks for all kernel images /usr/lib/modules/<kvers>/vmlinuz
     */
    ret = init_kernel_pkginfos_all(tool);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Update:
     * - pkg_vers
     * - vers_curr
     * - vers_prev
     * - managed
     */
    for (size_t i = 0; i < tool->num_pkgs; i++) {
        pkginfo = &tool->pkginfo[i];

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

