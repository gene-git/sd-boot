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

#include "sd-boot.h"

/**
 * Map kernel image to package name
 */
static int kernel_package_names(Triggers *trigs) {
    int ret = 0;

    if (trigs->num_info == 0) {
        return ret;
    }

    for (size_t i = 0; i < trigs->num_info; i++) {
        ret = kernel_image_path_to_info(&trigs->info[i]);
        if (ret != 0) {
            goto exit;
        }
    }
exit:
    return ret;
}

/**
 * Read all triggers from stdin from alpm hook
 * 2 kinds of triggers:
 * a) kernel specific triggers (/usr/lib/modules/<kern-vers>/vmlinuz
 *    gather relevant kernel info for these
 * b) any other trigger - these affact all kernels 
 *    For these keep a count how many not the trigger itself..
 *
 * Returns
 *  trigs->num_info
 *  trigs->info
 *  trigs->num_other
 *
 * Each info in the array is a KernelInfo:
 *  info->image
 *  info->mod_dir
 *  info->package
 *  info->vers
 */
int get_kernel_triggers(Triggers *trigs) {
    /*
     * Read all triggers from stdin.
     */
    int ret = 0;

    Array_str trigs_all = {};
    if (read_triggers(&trigs_all) != 0) {
        msg(MSG_ERR, "  ! sd-boot: error reading kernel triggers\n");
        ret = -1;
        goto exit;
    }

    /*
     * Parse them
     */
    ret = parse_kernel_triggers(&trigs_all, trigs);
    if (ret != 0) {
        goto exit;
    }

    /* 
     * Get kernel package name from kernel image path
     */
    if (trigs->num_info > 0) {
        ret = kernel_package_names(trigs);
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    array_str_free(&trigs_all);
    if (ret != 0) {
        free_triggers(trigs);
    }
    return ret;
}

void free_triggers(Triggers *trigs) {
    /*
     * Free up any mem
     */
    if (trigs->info != nullptr) {
        for (size_t i = 0; i < trigs->num_info; i++) {
            kernel_info_free(&trigs->info[i]);
        }
        free((void *)trigs->info);
        trigs->info = nullptr;
    }
    trigs->num_info = 0;
    trigs->num_info_alloc = 0;
    trigs->num_other = 0;
}

