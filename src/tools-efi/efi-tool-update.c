// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Standalone tool to update a bootable efi program.
 *
 * Usage:
 *  sd-boot-update-efi-tool <oper> <packge name> 
 *  - <oper> is one of: add, remove or inspect.
 *  - <package name> is arch package name.
 *
 * The package must be already installed (by pacman) in the usual way. 
 * This tool installs the program provided by the package into /boot (or /efi).
 */
#include <stdbool.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-msg.h"
#include "sd-boot-tool.h"

static void usage() {
    msg(MSG_ERR, "! sd-boot-efi-tool-update: Usage:\n");
    msg(MSG_ERR, "    sd-boot-efi-tool-update <oper> <package-name>\n");
    msg(MSG_ERR, "              <oper> = add or remove\n");
    msg(MSG_ERR, "      <package-name> = name of package to update\n");
}

/*
 * Returns true for operations supported here
 */
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
    int ret = 0;

    tool->triggers = false;
    ret = initialize_tool(SDB_EFI_TOOL, 3, argc, argv, tool);
    if (ret != 0) {
        usage();
        ret = 1;
        goto exit;
    }

    if (!operation_supported(tool->conf.oper)) {
        msg(MSG_ERR, "  ! sd-boot: unsupported opertion: %s\n", tool->conf.oper_str);
        ret = 1;
        goto exit;
    }

exit:
    return ret;
}


/*
 * Installs efi tool (such as efi-shell or memtest86) into efi
 *
 * Takes 2 argument:
 * - oper = add or remove or inspect
 *   operations are those offewred by systemd kernel-install.
 *
 * - package-name
 *   Special package name "--all--" applies to all kernels managed by sd-boot
 *
 * - add:
 *   Add the bootable efi image into the efi using kernel-install.
 *   Boot loader entry will be created following bls layout.
 *   If older items are found they are removed.
 * 
 * - inspect
 *   Shows install information about the package.
 *
 * - remove
 *   - remove efi tool from the efi (and any loader entries).
 *
 * Required data Files:
 *  - /etc/sd-boot/<package-name>.image
 *    Contains the path to the efi image file to be installed into boot_root.
 */
int main(int argc, char *argv[]) {
    int ret = 0;
    Tool tool = {};

    ret = initialize(argc, argv, &tool);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Check if any packages are managed by us
     * Check operation is supported.
     */
    if (!tool.managed) {
        goto exit;
    }

    /*
     * Do it
     */
    ret = efi_tool_update_execute(&tool);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

exit:
    tool_free(&tool);
    return ret;
}

