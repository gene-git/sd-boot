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
#include <string.h>

#include "sd-boot.h"

enum Const { BUF = 8 };


static void print_header() {
    msg(MSG_VERB, "%70s\n", "(c)urrent (a)ctive");
    msg(MSG_VERB, "%20s %-30s (%s %s) %s\n", "Device", "Mount", "c", "a", "type");
}

static char *marker(TriState state, char *t_str, char *f_str, char *u_str) {
    switch (state) {
        case True:
            return t_str;
            break;

        case False:
            return f_str;
            break;

        case Unknown:
        default:
            return u_str;
            break;
    }
}

static void print_row(char *ptype, MountInfo *mount) {
    char *active = marker(mount->active, "✔", " ", "?");
    char *current = marker(mount->current, "✔", " ", " ");

    msg(MSG_VERB, "%20s %-30s (%s %s) %s\n", mount->device, mount->mount, current, active, ptype);
}

int main() {
    /*
     * print the efi 
     */
    BootMounts boot_mounts = {};

    /*
     * Get current boot efi
     */
    print_header();
    if (find_boot_mounts(&boot_mounts) != 0) {
        msg(MSG_ERR, "Failed to locate EFI or xbootldr mount points\n");
        return 1;
    }

    for (size_t i = 0; i < boot_mounts.num_efis; i++) {
        print_row("EFI", &boot_mounts.efis[i]);
    }

    for (size_t i = 0; i < boot_mounts.num_xbootldrs; i++) {
        print_row("XBOOTLDR", &boot_mounts.xbootldrs[i]);
    }

    boot_mounts_free(&boot_mounts);

    return 0;
}
