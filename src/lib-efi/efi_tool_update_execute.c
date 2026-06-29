// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Install/remove/inspect a bootable efi program to/from/in $BOOT.
 *
 * - called by efi-tool-update[-triggers]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-efi.h"

/*
 * Install, remove or inspect an efi tool.
 *
 * Examples include: efi-shell and memtest86
 * Shared by efi-tool-update and efi-tool-update-triggers
 *
 * Does install / remove to / from the EFI.
 * Inspect prints detailed information.
 *
 * Sets up and runs "kernel-install" to do the real work.
 *
 * Args:
 * - Tool *tool
 */
int efi_tool_update_execute(Tool *tool) {

    int ret = 0;

    /*
     * Do it
     */
    for (size_t i = 0; i < tool->num_pkgs; i++) {
        PkgInfo *pkginfo = &tool->pkginfo[i];

        if (!pkginfo->managed) {
            continue;
        }

        ret = efi_tool_update_one(&tool->conf, pkginfo);
        if (ret != 0) {
            ret = 1;
            goto exit;
        }
    }

exit:
    return ret;
}

