// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Get list of updated PkgInfos from alpm trigger list.
 */

#include "sd-boot-msg.h"
#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-kernel.h"
#include "sd-boot-tool.h"
#include "sd-boot.h"

/*
 * Parsing the triggers is pushed to lib-efi for efi-tools and lib-kernel 
 * for kernels. Triggers can be packages or paths.
 *
 * For both efi-tools and kernels,  one specific item (efi-tool or kernel) is udpated
 * or all items (efi-tools or kernels).
 *
 * They are a little different in detail.
 * - efi-tools:
 *   Package triggers (one or more) cause that specific package to be updated.
 *   Any path trigger causes all efi-tools to be updated.
 *
 *- kernels:
 *  The /usr/lib/modules/<kern-version>/... path updates that one specific kernel.
 *  There are no kernel package triggers.
 *
 *  Any other trigger, path or package leads to all (managed) kernels to be updated.
 *  
 */
int pkginfos_from_triggers(TriggerInfo *triggers, Tool *tool) {
    int ret = 0;

    switch (tool->conf.tool_type) {
        case SDB_KERNEL:
            ret = kernel_pkginfos_from_triggers(triggers, tool);
            break;

        case SDB_EFI_TOOL:
            ret = efi_tool_pkginfos_from_triggers(triggers, tool);
            break;

        default:
            msg(MSG_ERR, "!  sd-boot warning update triggers - bad tool type %d\n", tool->conf.tool_type);
            break;
    }
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}
