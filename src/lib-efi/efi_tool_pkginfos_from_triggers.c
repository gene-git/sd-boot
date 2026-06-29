// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Generate the list of efi-tool PkgInfos given Triggers provided in TriggerInfo
 *
 * triggers originate from pacman ALPM hooks.
 * Each trigger is either a package name or pathname.
 *
 * 2 kinds of triggers:
 *
 * (a) package name to update
 *      Update any package name provided.
 *
 * (b) pathname
 *     update every managed efi-tool packages
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-tool.h"

/*
 * Any paths => (update) all managed efi-tools
 * packages  => (update) these packages.
 */
int efi_tool_pkginfos_from_triggers(TriggerInfo *tinfo, Tool *tool) {
    int ret = 0;
    PkgInfo *pkginfo = nullptr;

    if (tinfo->paths.num_rows > 0) {
        /*
         * All managed packages
         */
        ret = tool_alloc(tool->managed_pkgs.num_rows, tool);
        if (ret != 0) {
            goto exit;
        }

        for (size_t i = 0; i < tool->num_pkgs; i++) {
            pkginfo = &tool->pkginfo[i];
            pkginfo->pkg_name = strdup(tool->managed_pkgs.rows[i]);
            if (!pkginfo->pkg_name) {
                ret = -1;
                goto exit;
            }
        }

    } else if (tinfo->pkgs.num_rows > 0) {

        /*
         * Specified packages
         */
        ret = tool_alloc(tinfo->pkgs.num_rows, tool);
        if (ret != 0) {
            goto exit;
        }

        for (size_t i = 0; i < tool->num_pkgs; i++) {
            pkginfo = &tool->pkginfo[i];

            pkginfo->pkg_name = tinfo->pkgs.rows[i];
            tinfo->pkgs.rows[i] = nullptr;
        }
    }

    ret = pkginfos_update_from_pkg_name(tool);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}
