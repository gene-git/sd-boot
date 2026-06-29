// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Generate the efi-tool ki_vers string;
 *
 * ki_vers ~ <package-name>-<package-version>
 */

#include <stdio.h>
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-efi.h"

char *efi_tool_ki_vers(char *pkg_name, char *pkg_vers) {
    char version[EFI_ROW_MAX] = {};
    char *ki_vers = nullptr;

    if (!pkg_name || !pkg_vers) {
        return ki_vers;
    }

    if (snprintf(version, EFI_ROW_MAX-1, "%s-%s", pkg_name, pkg_vers) < 0) {
        msg(MSG_ERR, "  ! sd-boot: efi tool: error creating ki_vers: %s %s\n", pkg_name, pkg_vers);
        return ki_vers;
    }

    ki_vers = strdup(version);
    return ki_vers;
}

