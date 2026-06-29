// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Get list of updated PkgInfos from package name.
 *
 * conf must be loaded and conf->is_kernel / conf->is_efi_tool should be set.
 * - Parse command line arguments
 * - Set operation : conf->oper
 * - Save package info - one named package or all packages of same type:
 *   Command line can contain a single package name or --all-packages--
 * - For each package pkginfo is updated with the package:
 *   - name
 *   - version
 *   - image
 *   - mod_dir
 *   - is_sd_boot_managed
 *
 */
#include <stddef.h>
#include <stdlib.h>

#include "sd-boot-utils.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-kernel.h"
#include "sd-boot-package.h"
#include "sd-boot-tool.h"
#include "sd-boot.h"


/*
 * Update one pkginfo
 * Any package not installed is marked not managed.
 */
static int update_one(PkgInfo *pkginfo, Tool *tool) {
    int ret = 0;

    /*
     * Currently installed versions
     */
    ret = package_version_installed(&tool->conf, pkginfo);
    if (ret != 0) {
        ret = 0;
        pkginfo->managed = false;
        goto exit;
    }

    /* 
     * - vers_curr / vers_curr: current/previous versions
     *   efi-tools use package version and kernels use kern-vers
     * - okay if no info available.
     */
    if (read_package_version_file(&tool->conf, pkginfo) < 0) {
        ret = -1;
        goto exit;
    }

    /*
     * Rest of pkginfo
     */
    switch (tool->conf.tool_type) {
        case SDB_KERNEL:
            ret = kernel_pkginfo_from_pkg_name(tool, pkginfo);
            if (ret != 0) {
                goto exit;
            }
            break;

        case SDB_EFI_TOOL:
            ret = efi_tool_pkginfo(&tool->conf, pkginfo);
            if (ret != 0) {
                goto exit;
            }
            break;

        case SDB_EFIFS:
        default:
            break;
    }
exit:
    return ret;
}


/*
 * Have package name, gather the remaining pkginfo elements:
 * Input:
 *  pkginfos where each has been initialized with: 
 *  pkg_name, managed:
 *
 * Output fields udpated.
 * - pkg_vers
 * - ki_vers
 * - ki_image
 * - x mod_dir (kernels only)
 */
int pkginfos_from_pkg_name(char *pkg_name, Tool *tool) {
    int ret = 0;

    if (!pkg_name) {
        return 0;
    }
    /*
     * Initialize: pkg_name, managed.
     */
    ret = init_pkginfos_from_pkg_name(pkg_name, tool);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Update: pkg_vers, ki_vers, ki_image, mod_dir
     */
    ret = pkginfos_update_from_pkg_name(tool);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}


/*
 * Update each package info in the list:
 * Input:
 *  tool->pkginfo[i].pkg_name
 *
 * Output:
 *  tool->pkginfo[i].pkg_vers
 *  tool->pkginfo[i].managed
 *  tool->pkginfo[i].pkg_vers
 *  tool->pkginfo[i].ki_vers
 *  tool->pkginfo[i].ki_image
 *  x tool->pkginfo[i].mod_dir (kernels only)
 */  
int pkginfos_update_from_pkg_name(Tool *tool) {
    int ret = 0;
    Array_str *managed_pkgs = &tool->managed_pkgs;

    for (size_t i = 0; i < tool->num_pkgs; i++) {
        PkgInfo *pkginfo = &tool->pkginfo[i];

        pkginfo->managed = string_in_list(pkginfo->pkg_name, managed_pkgs->num_rows, managed_pkgs->rows);

        ret = update_one(pkginfo, tool);
        if (ret != 0) {
            goto exit;
        }
    }
exit:
    return ret;
}
