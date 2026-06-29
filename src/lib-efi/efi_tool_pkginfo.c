// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Given package name fill out some pkginfo data
 *
 * Output:
 *  - info->ki_image
 */

#include "sd-boot-msg.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot.h"


int efi_tool_pkginfo(SdBoot *conf, PkgInfo *info) {
    int ret = 0;

    if (!info || !info->pkg_name) {
        return 0;
    }

    info->ki_image = package_to_efi_image(conf, info->pkg_name);
    if (!info->ki_image) {
        msg(MSG_ERR, "  ! sd-boot: add efi tool: no efi image found\n");
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}
