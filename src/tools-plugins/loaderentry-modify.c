// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Kernel Install Plugin: 95-sd-boot-loaderentry-modify.install
 *
 * Change one loader entry:
 *
 * For kernel images updates entry:
 *  - title xxx        => kernel package name
 *
 * For efi tools updates entry:
 *  - title            => efi tool package name
 *  - linux <efi-file> => efi <efi-file>
 *  - options ..       => remove this line (these are kernel options only)
 */
#include <stddef.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-kernel.h"
#include "sd-boot-msg.h"

/*
 * main entry
 */
int main(int argc, const char *argv[]) {
    /*
     * Invoked by kernel-install.
     * Installed in: 
     *  /usr/lib/kernel/install.d/95-sd-boot-loaderentry-modify.install
     */
    int ret = 0;
    KIplugin plugin = {};
    SdBoot conf = {};

    /*
     * initialize config
     */
    if (load_config(&conf) != 0) {
        msg(MSG_ERR, "  - sd-boot: warning - no config file\n");
    }

    /*
     * initialize plugin provided by kernel-install
     */
    ret = plugin_init(argc, argv, &plugin);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot efi tool loaderentry plugin failed\n");
        ret = 0;
        goto exit;
    }

    /*
     * Modify loader entry only applicable to "add"
     */
    if (strcmp(plugin.command, "add") != 0) {
        goto exit;
    }

    /*
     * call appropriate function to do the work.
     */
    if (plugin.is_kernel) {
        conf.tool_type = SDB_KERNEL;
        ret = loaderentry_modify_kernel(&conf, &plugin);
    } else {
        conf.tool_type = SDB_EFI_TOOL;
        ret = loaderentry_modify_efi(&conf, &plugin);
    }
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot = problem updating loader entry\n");
        ret = 0;
    }

exit:
    config_clean(&conf);
    plugin_free(&plugin);

    return ret;
}
