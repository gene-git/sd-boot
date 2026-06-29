// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Free memory in PkgInfo
 */
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

void pkginfo_free(PkgInfo *info) {

    if (!info) {
        return;
    }

    if (info->pkg_name) {
        free((void *)info->pkg_name);
    }

    if (info->pkg_vers) {
        free((void *)info->pkg_vers);
    }

    if (info->vers_curr) {
        free((void *)info->vers_curr);
    }

    if (info->vers_prev) {
        free((void *)info->vers_prev);
    }

    if (info->ki_vers) {
        free((void *)info->ki_vers);
    }

    if (info->ki_image) {
        free((void *)info->ki_image);
    }

    if (info->mod_dir) {
        free((void *)info->mod_dir);
    }

    memset((void *)info, 0, sizeof(PkgInfo));

    info->managed = false;

}
