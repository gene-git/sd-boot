// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Parse list of Triggers
 * pacman's ALPM hooks contain list of triggers - when a trigger "changes" then
 * pacman writes each changed trigger to the invoked action (Exec) program's stdin.
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

enum Constants {
    CHUNK = 16,
};

/**
 * Check if already seen this kernel 
 */
static bool have_image(const char *image, Triggers *trigs) {
    bool have_it = false;

    if (image == nullptr) {
        return false;
    }

    for (size_t i = 0; i < trigs->num_info; i++) {
        if (strcmp(image, (const char *)trigs->info[i].image) == 0) {
            have_it = true;
            break;
        }
    }

    return have_it;
}

static int add_kern_to_list(char *trig, Triggers *trigs) {
    /*
     * Add kernel item to list
     */
    int ret = 0;

    void *tmp_ptr = nullptr;
    size_t count = 0;
    size_t unit = sizeof(KernelInfo);

    /*
     * skip duplicates
     */
    if (! have_image(trig, trigs)) {

        count = trigs->num_info++;

        if (trigs->num_info > trigs->num_info_alloc) {
            trigs->num_info_alloc += CHUNK;

            tmp_ptr = realloc((void *)trigs->info, trigs->num_info_alloc * unit);
            if (tmp_ptr == nullptr) {
                ret = -1;
                goto exit;
            }
            trigs->info = (KernelInfo *)tmp_ptr;
        }

        /*
         * trigger paths lack leading "/"
         */
        ret = path_add_slash(trig, & (trigs->info[count].image));
        if (ret != 0) {
            msg(MSG_ERR, "  ! sd-boot error adding slash to kernel path %s\n", trig);
            goto exit;
        }
    }

exit:
    return ret;
}

/**
 * Free unused excess mem
 */
static int free_unused(Triggers *trigs) {
    int ret = 0;

    if (trigs->num_info_alloc > trigs->num_info) {
        size_t unit = sizeof(KernelInfo);

        if (trigs->num_info == 0) {
            if (trigs->info != nullptr) {
                free((void *)trigs->info);
                trigs->info = nullptr;
            }
        } else {
            void *tmp_ptr = nullptr;
            tmp_ptr = realloc((void *) trigs->info, trigs->num_info * unit);
            if (tmp_ptr == nullptr) {
                ret = -1;
                goto exit;
            }
            trigs->info = (KernelInfo *) tmp_ptr;
        }
        trigs->num_info_alloc = trigs->num_info;
    }
exit:
    return ret;
}

/**
 * Parse all the triggers that were read in.
 * Input is array of strings - one for each trigger
 * Output is the Triggers *trigs.
 *
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
int parse_kernel_triggers(Array_str *trigs_all, Triggers *trigs) {
    int ret = 0;

    trigs->num_info = 0;
    if (trigs_all->num_rows == 0 || trigs_all->rows == nullptr) {
        msg(MSG_ERR, "  ! sd-boot: No kernel triggers passed in!\n");
        goto exit;
    }

    trigs->num_info_alloc = 0;
    size_t unit = sizeof(KernelInfo);

    trigs->info = (KernelInfo *)calloc(CHUNK, unit);
    if (trigs->info == nullptr) {
        ret = -1;
        goto exit;
    }
    trigs->num_info_alloc = CHUNK;

    /*
     * 2 kinds of triggers:
     * - kernel image - affects one specific kernel.
     * - any other - affects all kernels
     *   we only keep a count of these (ignore the trigger string)
     */
    bool is_kernel = false;

    for (size_t i = 0; i < trigs_all->num_rows; i++) {

        if (trigs_all->rows[i] == nullptr || trigs_all->rows[i][0] == '\0') {
            continue;
        }

        is_kernel = false;
        if (is_kernel_image_path(trigs_all->rows[i], &is_kernel) != 0) {
            /*
             * Should never happen - keep going.
             */
            continue;
        }

        /*
         * Develop testing - non production use.
         * msg(MSG_ERR, " trigger: %s (%d)\n", trigs_all->rows[i], is_kernel);
         */
        if (is_kernel) {
            if (add_kern_to_list(trigs_all->rows[i], trigs) != 0) {
                ret = -1;
                goto exit;
            }
        } else {
            trigs->num_other++;
        }
    }

    /*
     * Free up any excees mem
     */
    ret = free_unused(trigs);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}

