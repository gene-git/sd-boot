// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Install, remove or inspect a kernel in /efi or /boot.
 * Used by kernel-update and kernel-update-triggers.
 *
 * Sets up and runs "kernel-install" to do the real work.
 * See also man kernel-install.
 */
#include <stdbool.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-kernel.h"

/**
 * Operations:
 *
 * - add:
 *   - Install or update kernel into /efi or /boot
 *
 * - remove
 *   - remove kernel files from boot_root
 *
 * Required Files:
 *  a) /etc/sd-boot/kernel-install.packages
 *     List of kernel packages managed by sd-boot
 *  b) /usr/lib/modules/<kern-vers>/<pkgbase>
 *
 *    This file provides the package name of that kernel.
 *    <pkgbase> file will be looked for:
 *      pkgbase-sdb or pkgbase
 */
int kernel_update_execute(Tool *tool) {
    int ret = 0;

    /*
     * Do it
     */
    for (size_t i = 0; i < tool->num_pkgs; i++) {
        PkgInfo *pkginfo = &tool->pkginfo[i];

        if (!pkginfo->managed) {
            continue;
        }
        ret = kernel_update_one(&tool->conf, pkginfo);
        if (ret != 0) {
            ret = 1;
            goto exit;
        }
    }

exit:
    return ret;
}

