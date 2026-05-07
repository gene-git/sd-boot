// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read Triggers from stdin
 * Parse them
 */
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
        if (strcmp(image, (const char *)trigs->info[0].image) == 0) {
            have_it = true;
            break;
        }
    }

    return have_it;
}

static int add_kern_to_list(char **trig_p, Triggers *trigs) {
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
    if (! have_image(*trig_p, trigs)) {

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

        trigs->info[count].image = *trig_p;
        *trig_p = nullptr;
    }

exit:
    return ret;
}


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
 * Read all triggers from stdin from alpm hook
 * 2 kinds of triggers:
 * a) kernel specific triggers (/usr/lib/modules/<kern-vers>/vmlinuz
 *    gather relevant kernel info for these
 * b) any other trigger - these affact all kernels - keep a count 
 *    how many.
 */
int get_kernel_triggers(Triggers *trigs) {
    int ret = 0;

    /*
     * Read all triggers from stdin.
     */
    Array_str trigs_arr = {};
    if (read_triggers(&trigs_arr) != 0) {
        msg(MSG_ERR, "sd-boot: error reading kernel triggers\n");
        ret = -1;
        goto exit;
    }

    /*
     * Parse them
     */
    trigs->num_info = 0;

    if (trigs_arr.num_rows == 0) {
        msg(MSG_ERR, "sd-boot: No kernel triggers passed in!\n");
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
    const char *kern_key = "/usr/lib/modules/";
    const size_t key_len = strlen(kern_key);

    for (size_t i = 0; i < trigs_arr.num_rows; i++) {
        /*
         * add_kern_to_list takes pointer trigs_arr.rows[i]
         */
        if (strncmp(trigs_arr.rows[i], kern_key, key_len) == 0) {
            ret = add_kern_to_list(&trigs_arr.rows[i], trigs);
            if (ret != 0) {
                ret = -1;
                goto exit;
            }
        } else {
            trigs->num_other++;
        }
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

    /*
     * Free up any excees mem
     */
    ret = free_unused(trigs);
    if (ret != 0) {
        goto exit;
    }


exit:
    array_str_free(&trigs_arr);
    if (ret != 0) {
        free_triggers(trigs);
    }
    return ret;
}

void free_triggers(Triggers *trigs) {
    /*
    // *ree up any mem
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

