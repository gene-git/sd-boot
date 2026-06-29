// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Given pkginfo->pkg_name:
 * Input:
 *  pkg_info->pkg_name
 *
 * Output:
 *  pkginfo->image
 *  pkginfo->mod_dir
 *  pkkginfo->vers
 */

#include "sd-boot-kernel.h"
#include "sd-boot.h"

int kernel_pkginfo_from_pkg_name(Tool *tool, PkgInfo *info) {
    int ret = 0;

    ret = kernel_pkg_name_to_image(info->pkg_name, &info->ki_image);
    if (ret != 0) {
        goto exit;
    }

    ret = kernel_pkginfo_from_kernel_image(tool, info);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}
