// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Install or remove a bootable efi program into $BOOT.
 * - called by pacman alpm hook.
 * - Reads stdin to for the list of "triggers" where a trigger is the package name to update.
 *
 * Usage:
 *  sd-boot-efi-tool-update-triggers <oper>
 *  - <oper> is add or remove.
 *
 * Similar to sd-boot-efi-tool-update except triggers are read from stdin
 * instead of a package name on the command line.
 *
 * The package name is read from stdin.
 *
 * Example of an efi tool package is edk2-shell.
 * Supports SDB_DEV_TEST which sets BOOT_ROOT to the testing tree.
 */

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-msg.h"
#include "sd-boot-tool.h"


static bool operation_supported(Operation oper) {

    switch (oper) {
        case KI_ADD:
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

    tool->triggers = true;
    ret = initialize_tool(SDB_EFI_TOOL, 2, argc, argv, tool);
    if (ret != 0) {
        msg(MSG_ERR, "!  sd-boot: efi-tool triggers failed to initialize\n");
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


int main(int argc, char *argv[]) {
    /*
     * Install efi tool (like efi-shell or memtest86) into the efi.
     *
     * Takes one argument:
     * - <oper>
     *
     * - add:
     *   Add the bootable efi image using kernel-install into the efi..
     *   Boot loader entry will be created.
     *   If an older entry is present then it is removed
     *
     * - remove
     *   - remove an efi tool from the efi (also removed the loader entry)
     *
     * Triggers:
     *  - Provided by pacman from alpm hoooks and are read from stdin
     *
     * Input Files:
     *  - /etc/sd-boot/<package-name>.image
     *    Contains the path to the efi image file to be installed into boot_root.
     */

    int ret = 0;
    Tool tool = {};

    ret = initialize(argc, argv, &tool);
    if (ret != 0) {
        goto exit;
    }

    /*
     * True if any pkginfo packages are managed by us.
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

