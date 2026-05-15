// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Locate efi and (if exist) xbootldr partition mount points.
 *
 * With no args, displays efi and xbootldr mount pointsl
 * With one arg 
 *  - 'e(fi)' - display efi mount point
 *  - 'x(bootldr) display the xbootldr mount point
 *
 * NB this reports the "real" efi / xbootldr mount points as reported by "bootctl"
 * SDB_DEV_TEST / BOOT_ROOT play now role here.
 */

#include "sd-boot.h"

int main(int argc, const char *argv[]) {
    /*
     * print the efi 
     */
    int ret = 0;
    MountPoints mounts = {};

    ret = find_efi_xbootldr_mounts(&mounts);
    if (ret == 0) {
        if (argc < 2) {
            msg(MSG_VERB, "efi     : %s\n", mounts.efi_dir);
            msg(MSG_VERB, "xbootldr: %s\n", mounts.xbootldr_dir);
        } else {
            if (argv[0][0] == 'x') {
                msg(MSG_VERB, "%s\n", mounts.xbootldr_dir);
            } else {
                msg(MSG_VERB, "%s\n", mounts.efi_dir);
            }
        }
    } else {
        msg(MSG_ERR, "Failed to locate efi mount\n");
    }
    return 0;
}
