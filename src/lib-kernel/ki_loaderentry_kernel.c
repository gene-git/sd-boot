// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Change a loader entry for kernel:
 *
 * This updates:
 *  - title xxx        => kernel package name
 */
#include <stddef.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-kernel.h"
#include "sd-boot-msg.h"
#include "sd-boot-tool.h"
#include "sd-boot-utils.h"

int loaderentry_modify_kernel(SdBoot *conf, KIplugin *plugin) {
    /*
     * plugins are always called - each plugin decidws 
     * if input is something to act on or ignore.
     */
    int ret = 0;
    Array_str pkgs_arr = {};
    PkgInfo pkginfo = {};

    /*
     * Sanity check
     */
    if (!plugin->is_kernel || !plugin->kernel_version || plugin->kernel_version[0] == '\0') {
        goto exit;
    }

    /*
     * UKI layout has no loader entries
     * - conf->layout is from /etc/kernel/install.conf
     * - plugin->layout is from KERNEL_INSTALL_LAYOUT which could over-ride config setting.
     *   so use that.
     */
    if (plugin->is_uki) {
        goto exit;
    }

    /*
     * Get package name from the kernel image path
     */
    pkginfo.ki_image = strdup(plugin->kernel_image);
    ret = init_pkginfo_from_kernel_image(&pkginfo);
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
     * see also load_managed_package_list()
     */
    ret = read_managed_packages(conf, &pkgs_arr);
    if (ret != 0) {
        ret = 0;
        goto exit;
    }

    if (!string_in_list((const char *)pkginfo.pkg_name, pkgs_arr.num_rows, pkgs_arr.rows)) {
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
    entry.title = pkginfo.pkg_name;

    msg(MSG_NORMAL, "  ↳ sd-boot: updating kernel loader entry %s\n", pkginfo.pkg_name);

    ret = loaderentry_modify_file(&entry);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: Failed to update loader entry title: %s\n", pkginfo.pkg_name);
        ret = 1;
    }

exit:
    pkginfo_free(&pkginfo);
    array_str_free(&pkgs_arr);

    return ret;
}
