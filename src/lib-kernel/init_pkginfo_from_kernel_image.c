// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Kernel package installers (must) install a specific file into the module directory:
 *
 *   /usr/lib/modules/<kern-vers>/
 *
 * The file contains the name of the kernel package and filename must be one of:
 *     pkgbase-sdb, pkgbase
 *
 * For historical reasons it may also be named gc-pkgbase
 */
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "sd-boot-msg.h"
#include "sd-boot-utils.h"
#include "sd-boot-kernel.h"
#include "sd-boot.h"

/**
 * Kernel Module dir to kernel package name
 * Input:
 *      info->mod_dir
 * Returns:
 *      info->vers
 *      info->pkg_name
 * package is read from mod_dir/<pkgbase>
 */
int kernel_mod_dir_to_pkg_name(PkgInfo *info) {
    int ret = 0;

    if (!info->mod_dir) {
        ret = -1;
        goto exit;
    }

    if (info->pkg_name) {
        free((void *)info->pkg_name);
        info->pkg_name = nullptr;
    }

    const char *files[] = {
        "pkgbase-sdb",
        "pkgbase",
        "gc-pkgbase",
    };
    size_t num_files = sizeof(files) / sizeof(files[0]);

    char path[PATH_MAX] = {};
    char *str = nullptr;

    for (size_t i = 0 ; i < num_files; i++) {
        if (snprintf(path, PATH_MAX, "%s/%s", info->mod_dir, files[i]) < 0) {
            ret = -1;
            goto exit;
        }

        str = read_file_first_row(path);
        if (str) {
            info->pkg_name = str;
            str = nullptr;
            break;
        } 
    }

    if (!info->pkg_name) {
        /*
         * caller decides if this is error.
         */
        // msg(MSG_ERR, "sd-boot: no package provides pkgbase file for: %s\n", info->mod_dir);
        ret = 0;
        goto exit;
    }

exit:
    return ret;
}


/**
 * Kernel image to:
 * input:
 *  info->ki_image ~ /usr/lib/modules/<kern-vers>/vmlinuz
 *
 * Output updats 3 fields :
 * - info->pkg_name
 * - info->mod_dir
 * - info->ki_vers
 */
int init_pkginfo_from_kernel_image(PkgInfo *info) {
    int ret = 0;
    char *ki_image = nullptr;

    if (!info->ki_image) {
        ret = -1;
        goto exit;
    }

    /*
     * Make copy as dirname/basename modifies content
     */
    ki_image = strdup(info->ki_image);
    if (!ki_image) {
        ret = -1;
        goto exit;
    }


    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    char *mod_dir = dirname(ki_image);
    char *kern_vers = basename(ki_image);

    info->mod_dir = strdup(mod_dir);
    if (!info->mod_dir) {
        ret = -1;
        goto exit;
    }

    info->ki_vers = strdup(kern_vers);
    if (!info->ki_vers) {
        ret = -1;
        goto exit;
    }

    if (!info->pkg_name) {
        ret = kernel_mod_dir_to_pkg_name(info);
        if (ret != 0) {
            ret = -1;
            goto exit;
        }
    }

exit:
    if (ki_image) {
        free((void *)ki_image);
    }
    return ret;
}

