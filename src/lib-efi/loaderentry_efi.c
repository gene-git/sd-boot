// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Change a loader entry file for an efi tool:
 *
 * This updates:
 *  - title xxx        => kernel package name
 *  - linux <efi-file> => efi <efi-file> (bls layout)
 *  - uki <efi-file>   => efi <efi-file> (uki layout)
 *  - options          => remove (these are kenel options)
 */
#include <stddef.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-msg.h"
#include "sd-boot-tool.h"
#include "sd-boot-utils.h"

int loaderentry_modify_efi(SdBoot *conf, KIplugin *plugin) {
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
    if (plugin->is_kernel || !plugin->kernel_version || plugin->kernel_version[0] == '\0') {
        goto exit;
    }

    /*
     * Get package name from the image path (pkginfo)
     */
    pkginfo.pkg_name = efi_image_to_package(conf, plugin->kernel_image);
    if (!pkginfo.pkg_name || pkginfo.pkg_name[0] == '\0') {
        // msg(MSG_VERB, "sd-boot: Failed to get kernel package from image: %s\n", plugin->kernel_image);
        ret = 0;
        goto exit;
    }

    /*
     * Get list of efi tool packages managed by sd-boot
     * and check this package is listed.
     *
     * If no managed packages - then nothing for us to do.
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
    entry.is_kernel = false;
    entry.is_efi_tool = true;
    entry.loader_entry_dir = plugin->loader_entry_dir;
    entry.loader_entry_file = plugin->loader_entry_file;
    entry.title = pkginfo.pkg_name;

    msg(MSG_NORMAL, "  ↳ sd-boot: updating efi tool loader entry %s\n", pkginfo.pkg_name);

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
