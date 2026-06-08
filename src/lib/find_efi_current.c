// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Locate the Mount Points for:
 *   EFI (where ESP is mounted)
 *   XBOOTLDR (if there is an XBOOTLDR partition)
 *
 * Parses output of systemd's bootctl.
 */
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"


int find_efi_current_boot(MountPoints *mounts) {
    /*
     * Locate the ESP mount point (efi) for current boot
     * With multiple disks, there can be multiple ESPs some/all may be mounted.
     */ 
    int ret = 0;
    BootMounts boot_mounts = {};

    if (!mounts) {
        return 1;
    }

    mounts->efi_dir[0] = '\0';
    mounts->xbootldr_dir[0] = '\0';

    ret = find_boot_mounts(&boot_mounts);
    if (ret != 0) {
        goto exit;
    }

    for (size_t i = 0; i < boot_mounts.num_efis; i++) {
        if (boot_mounts.efis[i].active) {
            (void)strncpy(mounts->efi_dir, boot_mounts.efis[i].mount, PATH_MAX);
            break;
        }
    }


    for (size_t i = 0; i < boot_mounts.num_xbootldrs; i++) {
        if (boot_mounts.xbootldrs[i].active) {
            (void)strncpy(mounts->xbootldr_dir, boot_mounts.xbootldrs[i].mount, PATH_MAX);
            break;
        }
    }

exit:
    boot_mounts_free(&boot_mounts);
    return ret;
}
