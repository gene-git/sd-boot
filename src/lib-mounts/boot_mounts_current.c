// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Locate the Mount Points for:
 *   EFI (where ESP is mounted)
 *   XBOOTLDR (if there is an XBOOTLDR partition)
 *
 * Parses output of systemd's bootctl.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-config.h"
#include "sd-boot-mounts.h"


int boot_mounts_mark_current(SdBoot *conf, BootMounts *boot_mounts) {
    /*
     * ESP and XBOOTLDR can be current (determined by bootctl)
     * or active (used to boot but not current).
     * With multiple disks, there can be multiple ESPs / XBOOTLDRs
     *
     * At least one of efi and xbootldr must be non-null for which info
     * will be returned.
     *
     * Caller must free content.
     */ 
    int ret = 0;
    MountInfo efi = {};
    MountInfo xbootldr = {};

    if (!boot_mounts) {
        return 1;
    }

    ret = find_boot_mounts_current(conf, &efi, &xbootldr);
    if (ret != 0) {
        ret = -1;
        goto exit;
    }

    MountInfo *this = nullptr;
    if (efi.mount) {
        for (size_t i = 0; i < boot_mounts->num_efis; i++) {
            this = &boot_mounts->efis[i];
            if (this->mount && strcmp(this->mount, efi.mount) == 0) {
                this->current = True;
                break;
            }
        }
    }

    if (xbootldr.mount) {
        for (size_t i = 0; i < boot_mounts->num_xbootldrs; i++) {
            this = &boot_mounts->xbootldrs[i];
            if (this->mount && strcmp(this->mount, xbootldr.mount) == 0) {
                this->current = True;
                break;
            }
        }
    }

exit:
    if (ret != 0) {
        msg(MSG_ERR, "  sd-boot error retrieving current boot mounts\n");
    }
    mount_info_free(&efi);
    mount_info_free(&xbootldr);
    return ret;
}
