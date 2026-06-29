// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Initialize list of pkginfos each has set:
 * - pkginfo->pkg_name
 * - pkginfo->managed
 * 
 * The remaining elements can be updated using:
 * - pkginfos_from_pkg_name()
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-utils.h"
#include "sd-boot.h"


static int init_pkginfo_one(char *pkg_name, Tool *tool) {
    int ret = 0;
    PkgInfo *pkginfo = nullptr;
    Array_str *managed_pkgs = &tool->managed_pkgs;

    ret = tool_alloc(1, tool);
    if (ret != 0) {
        goto exit;
    }

    pkginfo = &tool->pkginfo[0];

    if (pkginfo->pkg_name) {
        free((void *)pkginfo->pkg_name);
        pkginfo->pkg_name = nullptr;
    }

    pkginfo->pkg_name = strdup(pkg_name);
    if (!pkginfo->pkg_name) {
        ret = -1;
        goto exit;
    }

    pkginfo->managed = string_in_list(pkg_name, managed_pkgs->num_rows, managed_pkgs->rows);

exit:
    return ret;
}


int init_pkginfos_all_managed(Tool *tool) {
    int ret = 0;
    size_t num_pkgs = 0;
    Array_str *managed_pkgs = &tool->managed_pkgs;
    num_pkgs = managed_pkgs->num_rows;

    ret = tool_alloc(num_pkgs, tool);
    if (ret != 0) {
        goto exit;
    }

    for (size_t i = 0; i < num_pkgs; i++) {
        PkgInfo *pkginfo = &tool->pkginfo[i];

        pkginfo->managed = true;
        pkginfo->pkg_name = strdup(managed_pkgs->rows[i]);
        if (!pkginfo->pkg_name) {
            ret = -1;
            goto exit;
        }
    }

exit:
    return ret;
}



int init_pkginfos_from_pkg_name(char *pkg_name, Tool *tool) {
    int ret = 0;

    /*
     * Get list of all package names either:
     * - pkg_name -> get pkginfo for this package
     * - pkgname == "--all--" => get all pkginfos
     */
    if (strcmp(pkg_name, "--all--") == 0) {

        ret = init_pkginfos_all_managed(tool);
        if (ret != 0) {
            goto exit;
        }

    } else {

        ret = init_pkginfo_one(pkg_name, tool);
        if (ret != 0) {
            goto exit;
         }
    }

exit:
    return ret;
}

