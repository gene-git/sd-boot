// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Find the kernel image path from the 
 */
#include <libgen.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-kernel.h"
#include "sd-boot-utils.h"

int kernel_pkg_name_to_image(char *pkg_name, char **ki_image_p) {
    int ret = 0;
    PkgInfo info_tmp = {};
    Array_str image_paths = {};
    char *ki_image = nullptr;

    if (!pkg_name) {
        goto exit;
    }

    ret = get_all_kernel_image_paths(&image_paths);
    if (ret != 0) {
        goto exit;
    }

    if (image_paths.num_rows == 0) {
        goto exit;
    }

    /*
     * Check every kernel image file for <pkgbase> containing pkg_name
     * match pkg_name to identofy the matching kernel image
     */
    for (size_t i = 0; i < image_paths.num_rows; i++) {
        
        ki_image = strdup(image_paths.rows[i]);
        if (!ki_image) {
            ret = -1;
            goto exit;
        }

        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        info_tmp.mod_dir = dirname(ki_image);
        ret = kernel_mod_dir_to_pkg_name(&info_tmp);
        if (ret != 0 || !info_tmp.pkg_name) {
            goto exit;
        }

        info_tmp.mod_dir = nullptr;
        free((void *)ki_image);
        ki_image = nullptr;

        if (strcmp(pkg_name, info_tmp.pkg_name) == 0) {
            free((void *)info_tmp.pkg_name);
            info_tmp.pkg_name = nullptr;
            *ki_image_p = strdup(image_paths.rows[i]);
            break;
        }

        free((void *)ki_image);
        ki_image = nullptr;

        free((void *)info_tmp.pkg_name);
        info_tmp.pkg_name = nullptr;
        info_tmp.mod_dir = nullptr;
    }

exit:
    if (ki_image) {
        free((void *)ki_image);
    }
    if (info_tmp.pkg_name) {
        free((void *)info_tmp.pkg_name);
    }
    array_str_free(&image_paths);

    return ret;
}

