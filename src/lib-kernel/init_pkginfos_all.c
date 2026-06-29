// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Get a list of all kernel image paths matching:
 * - /usr/lib/modules/<kern-vers>vmlinuz
 * - init pkginfo for each
 *
 * calls init_pkginfo_from_kernel_image() so each pkginfo has 
 * initialized the elements:
 * - info->pkg_name
 * - info->ki_image
 * - info->ki_vers
 * - info->mod_dir
 *
 * Fields not updated (see kernel_pkginfos_all()):
 *
 * - info->pkg_vers
 * - info->vers_curr
 * - info->vers_prev
 * - info->managed
 *
 * Allocate an array of PkgInfo structs.
 * Caller must free each PkgInfo and the array.
 *
 * Internal to lib-kernel
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-kernel.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"


int init_kernel_pkginfos_all(Tool *tool) {
    int ret = 0;
    Array_str image_paths = {};
    PkgInfo *pkginfo = nullptr;

    /*
     * Get file list
     */
    tool->num_pkgs = 0;
    ret = get_all_kernel_image_paths(&image_paths);
    if (ret != 0) {
        goto exit;
    }

    if (image_paths.num_rows == 0) {
        goto exit;
    }

    /*
     * kernel info from module path
     */
    tool->num_pkgs = image_paths.num_rows;
    tool->pkginfo = (PkgInfo *)calloc(tool->num_pkgs, sizeof(PkgInfo));
    if (!tool->pkginfo) {
        ret = -1;
        goto exit;
    }

    for (size_t i = 0; i < tool->num_pkgs; i++) {

        pkginfo = &tool->pkginfo[i];

        pkginfo->ki_image = image_paths.rows[i];
        image_paths.rows[i] = nullptr;

        ret = kernel_pkginfo_from_kernel_image(tool, pkginfo);
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    array_str_free(&image_paths);
    return ret;
}
