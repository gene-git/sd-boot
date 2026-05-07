// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Get a list of all kernel image paths matching:
 * - /usr/lib/modules/<kern-vers>vmlinuz
 * - update kernel info for each 
 *
 * Allocate an array of KernelInfo structs.
 * Caller must free each KernelInfo and the array.
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

int kernel_info_all(size_t *num_info_p, KernelInfo **info_p) {
    int ret = 0;
    int sret = 0;
    const char *pattern = "/usr/lib/modules/*/vmlinuz";
    Array_str files = {};

    /*
     * Get file list
     */
    *num_info_p = 0;
    ret = file_list_glob(pattern, &files);
    if (ret != 0) {
        goto exit;
    }

    if (files.num_rows == 0) {
        goto exit;
    }

    /*
     * kernel info from module path
     */
    *num_info_p = files.num_rows;
    *info_p = (KernelInfo *)calloc(*num_info_p, sizeof(KernelInfo));
    if (*info_p == nullptr) {
        ret = -1;
        goto exit;
    }

    for (size_t i = 0; i < *num_info_p; i++) {
        (*info_p)[i].image = files.rows[i];
        files.rows[i] = nullptr;
        sret = kernel_image_path_to_info(&(*info_p)[i]);
        if (sret != 0) {
            msg(MSG_ERR, "sd-boot: Error gather kernel info: %s", (*info_p)[i].image);
            ret = -1;
            goto exit;
        }
    }

exit:
    array_str_free(&files);

    if (ret != 0 &&  *num_info_p > 0 && *info_p != nullptr) {
       /*
        * if error => Clean up
        */
        for (size_t i = 0; i < *num_info_p; i++) {
            kernel_info_free(&(*info_p)[i]);
        }
        free((void *)*info_p);
    }
    return ret;
}
