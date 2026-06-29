// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Generate the list of kernel PkgInfos given Triggers provided in TriggerInfo
 *
 * pacman's ALPM hooks contain list of triggers - when a trigger "changes" then
 * pacman writes each changed trigger to the invoked action (Exec) program's stdin.
 */
#include <stdbool.h>

#include "sd-boot-kernel.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"

/**
 * 
 * 2 kinds of triggers:
 * a) kernel specific triggers (/usr/lib/modules/<kern-vers>/vmlinuz
 *    gather relevant kernel info for these
 *
 * b) any other trigger - package or path - these affact all kernels 
 *    For these keep a count how many not the trigger itself..
 *
 * Approach:
 * - check for any package name triggers - 
 *   - if found - update all kernels
 *   - if no package names, check all path based triggers:
 *     - check if any kernel image path triggers.
 *     - if kernel images are the only triggers, 
 *       update those kernels, 
 *     - if any other path triggers then update all kernels.
 *
 * Note:
 *    A kernel package name must never be used as a trigger [see (a)].
 *    Trigger paths do not have leading "/"
 *
 * Returns
 *  tool->pkginfo
 *
 * Each info in the array has:
 *  info->pkg_name
 *  info->pkg_vers
 *  info->ki_image
 *  info->mod_dir
 *  info->vers_curr
 *  info->vers_prev
 */

int kernel_pkginfos_from_triggers(TriggerInfo *tinfo, Tool *tool) {
    int ret = 0;

    /*
     * Parse them
     */
    ret = parse_kernel_triggers(tinfo);
    if (ret != 0) {
        goto exit;
    }

    if (tinfo->pkgs_special.num_rows > 0 || tinfo->paths_other.num_rows > 0) {
        /*
         * All kernels if triggers contain:
         * - any package or any non-kernel-image module
         */
        ret = kernel_pkginfos_all(tool);
        if (ret != 0) {
            goto exit;
        }

    } else if (tinfo->paths_special.num_rows > 0) {
        /*
         * Specific kernels if:
         * - kernel image path (i.e. tinfo->paths_special)
         */
        ret = kernel_pkginfos_from_image_paths(&tinfo->paths_special, tool);
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    return ret;
}

