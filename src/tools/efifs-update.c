// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Updates the efi filesystem drivers by copying from
 * /usr/lib/efifs-x64/ ==> "$efi"/EFI/systemd/drivers
 *
 */

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-efifs.h"
#include "sd-boot-msg.h"
#include "sd-boot-tool.h"

/*
 * Takes one argument:
 * - add or remove
 *
 * Locate efi
 * - add:
 *   - copy all the file system drivers from
 *   /usr/lib/efifs-x64/ => <efi>//EFI/systemd/drivers/
 *
 * - remove
 *   - remove fles from <efi>//EFI/systemd/drivers/
 *
 * Triggers:
 *  - none
 * 
 * Required Input Files:
 *  - none
 */
int main(int argc, char *argv[]) {
    int ret = 0;
    Tool tool = {};

    tool.triggers = false;
    ret = initialize_tool(SDB_EFIFS, 2, argc, argv, &tool);
    if (ret != 0) {
        msg(MSG_ERR, "! sd-boot: missing add or remove\n");
        ret = 1;
        goto exit;
    }

    ret = efifs_add_remove(&tool.conf);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

exit:
    if (ret != 0 && !tool.conf.efivars_available) {
        ret = 0;
    }
    tool_free(&tool);
    return ret;
}

