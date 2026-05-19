// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Locate ESP mount points (efi) and (if exist) any xbootldr partition mount points.
 *
 * NB this marks current efi / xbootldr mount points as reported by "bootctl" with 
 * asterisk.
 */
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>

#include "sd-boot.h"

enum Const { BUF = 8 };

int main() {
    /*
     * print the efi 
     */
    int ret = 0;
    Array_str efis = {};
    Array_str xbts = {};
    MountPoints mounts = {};
    
    /*
     * Warn if not root
     */
    if (geteuid() != 0) {
        msg(MSG_ERR, "! Warning: Must run as root\n");
    }

    /*
     * Get current boot efi
     */
    (void)find_efi_current_boot(&mounts);

    char *efi_curr = mounts.efi_dir;
    char *xbt_curr = mounts.xbootldr_dir;

    ret = find_efi_xbootldr_mounts(&efis, &xbts);
    if (ret == 0) {
        msg(MSG_VERB, "EFI:\n");
        for (size_t i = 0; i < efis.num_rows; i++) {
            if (efi_curr[0] != '\0' && strcmp(efi_curr, efis.rows[i]) == 0) {
                msg(MSG_VERB, "\t* %s\n", efis.rows[i]);
            } else {
                msg(MSG_VERB, "\t  %s\n", efis.rows[i]);
            }
        }

        msg(MSG_VERB, "XBOOTLDR:\n");
        for (size_t i = 0; i < xbts.num_rows; i++) {
            if (xbt_curr[0] != '\0' && strcmp(xbt_curr, xbts.rows[i]) == 0) {
                msg(MSG_VERB, "\t* %s\n", xbts.rows[i]);
            } else {
                msg(MSG_VERB, "\t  %s\n", xbts.rows[i]);
            }
        }

    } else {
        msg(MSG_ERR, "Failed to locate EFI or xbootldr mount points\n");
    }

    array_str_free(&efis);
    array_str_free(&xbts);

    return 0;
}
