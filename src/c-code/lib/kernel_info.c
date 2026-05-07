// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Kernel installers (must) install a file into the module directory:
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

#include "sd-boot.h"

/**
 * Kernel Module dir to kernel package name
 * Input:
 *      info->mod_dir
 * Returns:
 *      info->vers
 *      info->package
 * package is read from mod_dir/<pkgbase>
 */
static int kernel_mod_dir_to_package(KernelInfo *info) {
    int ret = 0;

    if (info->mod_dir == nullptr) {
        ret = -1;
        goto exit;
    }

    const char *files[] = {
        "gc-pkgbase",
        "pkgbase-sdb",
        "pkgbase",
    };
    size_t num_files = sizeof(files) / sizeof(files[0]);

    char path[PATH_MAX] = {'\0'};
    char *str = nullptr;

    for (size_t i = 0 ; i < num_files; i++) {
        if (snprintf(path, PATH_MAX, "%s/%s", info->mod_dir, files[i]) < 0) {
            ret = -1;
            goto exit;
        }

        str = read_file_first_row(path);
        if (str != nullptr) {
            info->package = str;
            str = nullptr;
            break;
        } 
    }
    if (info->package == nullptr) {
        /*
         * caller decides if this is error.
         * plugins are called always and its up to plugin to ignore or not
         * things it is not responsible for.
         * e.g. a kernel loader entry plugin gets called with efi-tool image.
         * not a kernel - so no package.
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
 *  info->image ~ /usr/lib/modules/<kern-vers>/vmlinuz
 *
 * Returns:
 * - info->mod_dir
 * - info->package
 * - info->vers
 */
int kernel_image_path_to_info(KernelInfo *info) {
    int ret = 0;

    char *img_path = nullptr;
    if (info->image == nullptr) {
        ret = -1;
        goto exit;
    }

    /*
     * Make copy since dirname modifies the data
     */
    img_path = strdup(info->image);
    if (img_path == nullptr) {
        ret = -1;
        goto exit;
    }

    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    char *mod_dir = dirname(img_path);
    char *kern_vers = basename(img_path);

    info->mod_dir = strdup(mod_dir);
    if (info->mod_dir == nullptr) {
        ret = -1;
        goto exit;
    }
    info->vers = strdup(kern_vers);
    if (info->vers == nullptr) {
        ret = -1;
        goto exit;
    }

    ret = kernel_mod_dir_to_package(info);
    if (ret != 0) {
        ret = -1;
        goto exit;
    }

exit:
    if (img_path != nullptr) {
        free((void *)img_path);
    }
    return ret;
}


/**
 * Free any KernelInfo mem
 */
void kernel_info_free(KernelInfo *info) {
    if (info->image != nullptr) {
        free((void *)info->image);
    }

    if (info->mod_dir != nullptr) {
        free((void *)info->mod_dir);
    }

    if (info->package != nullptr) {
        free((void *)info->package);
    }
    if (info->vers != nullptr) {
        free((void *)info->vers);
    }
}
