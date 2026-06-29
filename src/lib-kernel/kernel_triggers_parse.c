// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read Triggers from stdin
 * Parse them
 *
 * pacman's ALPM hooks contain list of triggers - when a trigger "changes" then
 * pacman writes each changed trigger to the invoked action (Exec) program's stdin.
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-kernel.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"


/**
 * Triggers are passed in
 *  trigger_info->pkgs
 *  trigger_info->paths
 *
 * General approach:
 *
 * (a) if any package name triggers - save one or more to trigger_info->pkgs_special 
 *     If find any - stop parsing as all kernels will be updated.
 *
 * (b) Check for kernel module triggers - save to  trigger_info->paths_special.
 *     each of these kernels will be udpated 
 *
 * (c) if find any non-kernel module path triggers (save to trigger_info->paths_other)
 *     stop - as this will update all kernels.
 *
 * Note that other than kernel module paths, we only are about counts being > 0;
 * Caller will:
 *
 *  if trigger_info->pkgs_special.num_rows > 0 ==> update all kernels
 *  if trigger_info->paths_other.num_rows  > 0 ==> update all kernels
 *  if trigger_info->paths_special.num_rows > 0 update those kernels
 *
 */
int parse_kernel_triggers(TriggerInfo *tinfo) {
    int ret = 0;
    size_t num_paths_special = 0;
    size_t num_paths_other = 0;

    /*
     * (a) Any packages -> update all
     */
    if (tinfo->pkgs.num_rows > 0) {
        ret = array_str_move(&tinfo->pkgs, &tinfo->pkgs_special);
        if (ret != 0) {
            goto exit;
        }
        return 0;
    }

    /*
     * (b) kernel module path / other paths
     */
    if (tinfo->paths.num_rows == 0) {
        return 0;
    }

    bool is_kernel_module = false; 
    for (size_t i = 0; i < tinfo->paths.num_rows; i++) {
        is_kernel_module = false; 
        is_kernel_image_path(tinfo->paths.rows[i], &is_kernel_module);

        if (is_kernel_module) {
            ret = array_str_resize(num_paths_special + 1, &tinfo->paths_special);

            tinfo->paths_special.rows[num_paths_special++] = tinfo->paths.rows[i];
            tinfo->paths.rows[i] = nullptr;

        } else {
            ret = array_str_resize(num_paths_other + 1, &tinfo->paths_other);

            tinfo->paths_other.rows[num_paths_other++] = tinfo->paths.rows[i];
            tinfo->paths.rows[i] = nullptr;

        }

        /*
         * (c) - non-kernel-modeul path - update all kernels.
         */
        if (num_paths_other > 0) {
            break;
        }
    }

exit:
    return ret;
}

