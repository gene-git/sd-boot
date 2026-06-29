// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Standalone tool to invoke kernel-install for an installed kernel package.
 *
 * Usage:
 *  sd-boot-update-kernel <oper> <package-name>
 *  - <oper> is one of: add, remove or inspect.
 *  - <package-name> is arch kernel package name.
 *
 * The kernel package must be already installed by pacman.
 * This tool installs the program provided by the package into /efi or /boot.
 *
 * See also man kernel-install.
 */
#include <stdbool.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-kernel.h"
#include "sd-boot-msg.h"
#include "sd-boot-tool.h"


static void usage() {
    msg(MSG_ERR, "! sd-boot-kernel-update: Usage: \n");
    msg(MSG_ERR, "    sd-boot-kernel-update <oper> <package-name>\n");
    msg(MSG_ERR, "              <oper> = add or remove\n");
    msg(MSG_ERR, "      <package-name> = name of package to update\n");
}

static bool operation_supported(Operation oper) {

    switch (oper) {
        case KI_ADD:
        case KI_INSPECT:
        case KI_REMOVE:
            return true;
            break;

        default:
            return false;
            break;
    }
}


static int initialize(int argc, char *argv[], Tool *tool) {
    /*
     * Command line & Config
     */
    int ret = 0;

    tool->triggers = false;
    ret = initialize_tool(SDB_KERNEL, 3, argc, argv, tool);
    if (ret != 0) {
        usage();
        ret = 1;
        goto exit;
    }

    if (!operation_supported(tool->conf.oper)) {
        msg(MSG_ERR, "  ! sd-boot: unsupported operation: %s\n", tool->conf.oper_str);
        ret = 1;
        goto exit;
    }

exit:
    return ret;
}


/**
 *
 * 2 command line args:
 *  operation package-name
 *
 * Operations:
 *
 * - add:
 *   Install or update kernel into /efi or /boot
 *
 * - remove
 *   remove kernel files from boot_root
 *
 * - inspect
 *   display information about the kernel-package provided on command line.
 *
 * Package Name:
 *   The name of a kernel package. 
 *   Or a special package name "--all--" means all kernels managed by sd-boot
 *
 *  (a) /etc/sd-boot/kernel-install.packages
 *      List of kernel packages managed by sd-boot
 *
 *  (b) /usr/lib/modules/<kern-vers>/<pkgbase>
 *      This file contains the package name of that kernel.
 *      <pkgbase> can be: pkgbase-sdb or pkgbase
 */
int main(int argc, char *argv[]) {
    int ret = 0;
    Tool tool = {};

    ret = initialize(argc, argv, &tool);
    if (ret != 0) {
        goto exit;
    }

    /*
     * true if any pkg managed by us
     */
    if (!tool.managed) {
        goto exit;
    }

    /*
     * Do it
     */
    ret = kernel_update_execute(&tool);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

exit:
    tool_free(&tool);
    return ret;
}

