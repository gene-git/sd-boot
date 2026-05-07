// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Change one loader entry for kernel:
 *
 * For kernel images this updates:
 *  - title xxx        => kernel package name
 */
#include <stddef.h>
#include <string.h>

#include "sd-boot.h"

int loaderentry_modify_kernel(SdBoot *conf, KIplugin *plugin) {
    /*
     * plugins are always called - each plugin decidws 
     * if input is something to act on or ignore.
     */
    int ret = 0;
    Array_str pkgs_arr = {};
    KernelInfo info = {};

    /*
     * Sanity check
     */
    if (! plugin->is_kernel || plugin->kernel_version == nullptr || plugin->kernel_version[0] == '\0') {
        goto exit;
    }

    /*
     * Get package name from the kernel image path (info)
     */
    info.image = strdup(plugin->kernel_image);
    ret = kernel_image_path_to_info(&info);
    if (ret != 0) {
        // msg(MSG_VERB, "sd-boot: Failed to get kernel package from image: %s\n", plugin.kernel_image);
        ret = 0;
        goto exit;
    }

    /*
     * Get list of kernel packages managed by sd-boot
     * and check if this package is listed.
     *
     * If no managed packages - then nothing for us to do.
     */
    ret = load_kernel_packages(conf, &pkgs_arr);
    if (ret != 0) {
        ret = 0;
        goto exit;
    }

    if (!string_in_list(info.package, pkgs_arr.num_rows, pkgs_arr.rows)) {
        goto exit;
    }

    /*
     * All good - update loader entry
     */
    LoaderEntry entry = {};
    entry.is_kernel = true;
    entry.is_efi_tool = false;
    entry.loader_entry_dir = plugin->loader_entry_dir;
    entry.loader_entry_file = plugin->loader_entry_file;
    entry.title = info.package;

    msg(MSG_NORMAL, "  sd-boot: updating kernel loader entry %s\n", info.package);

    ret = loaderentry_modify_file(&entry);
    if (ret != 0) {
        msg(MSG_ERR, "  sd-boot: Failed to update loader entry title: %s\n", info.package);
        ret = 1;
    }

exit:
    kernel_info_free(&info);
    array_str_free(&pkgs_arr);

    return ret;
}
