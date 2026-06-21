// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Container for ESP and XBOOTLDR mount points.
 */ 
#include <stdlib.h>

#include "sd-boot-mounts.h"

static void free_content(size_t num_0, size_t num_1, MountInfo *mount) {

    if (!mount) {
        return;
    }

    for (size_t i = num_0; i < num_1; i++) {
        mount_info_free(&mount[i]);
    }
}

/*
 * Alloc one of efis / xbootlrds.
 * Input is 
 * - num_new = new number of elements in array.
 * - num_p = address of number of elems in array.
 * - address of array of BountMount
 */
static int realloc_one(size_t num_new, size_t *num_p, MountInfo **mounts_p) {
    void *ptr = nullptr;
    MountInfo *mounts = nullptr;

    if (!mounts_p || !num_p) {
        return -1;
    }

    mounts = *mounts_p;
    if (num_new == *num_p) {
        return 0;
    }

    if (num_new == 0) {
        if (mounts) {
            free_content(0, *num_p, mounts);
            free((void *)mounts);
        }
        *mounts_p = nullptr;
        *num_p = num_new;

    } else if (num_new < *num_p) { 

        free_content(num_new, *num_p, mounts);

        ptr = realloc(mounts, num_new * sizeof(MountInfo));
        if (!ptr) {
            *num_p = 0;
            return -1;
        }
        *mounts_p = ptr;
        *num_p = num_new;

    } else {
        ptr = realloc(mounts, num_new * sizeof(MountInfo));
        if (!ptr) {
            *num_p = 0;
            return -1;
        }
        *mounts_p = ptr;
        mounts = *mounts_p;
        for (size_t i = *num_p; i < num_new; i++) {
            mounts[i].device = nullptr;
            mounts[i].mount = nullptr;
            mounts[i].active = Unknown;
            mounts[i].current = Unknown;
        }
        *num_p = num_new;
    }

    return 0;
}

int boot_mounts_alloc(size_t num_efis, size_t num_xbootldrs, BootMounts *boot_mounts) {
    int ret = 0;

    /*
     * efi
     */
    if (num_efis != boot_mounts->num_efis) {
        ret = realloc_one(num_efis, &boot_mounts->num_efis, &boot_mounts->efis);
        if (ret != 0) {
            return ret;
        }
    }

    /*
     * xbootldr
     */
    if (num_xbootldrs != boot_mounts->num_xbootldrs) {
        ret = realloc_one(num_xbootldrs, &boot_mounts->num_xbootldrs, &boot_mounts->xbootldrs);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

void boot_mounts_free(BootMounts *mounts) {

    if (!mounts) {
        return;
    }

    if (mounts->efis) {
        free_content(0, mounts->num_efis, mounts->efis);
        mounts->num_efis = 0;
        if (mounts->efis) {
            free((void *)mounts->efis);
        }
        mounts->efis = nullptr;
    }

    if (mounts->xbootldrs) {
        free_content(0, mounts->num_xbootldrs, mounts->xbootldrs);
        mounts->num_xbootldrs = 0;
        if (mounts->xbootldrs) {
            free((void *)mounts->xbootldrs);
        }
        mounts->xbootldrs = nullptr;
    }
}

/*
 * Allocate and copy 'this_mount' into the appropriate slot.
 * If 'is_esp' then saves to mounts->efis otherwise to 
 * mounts->xbootldrs. 
 * Returns 0 on success. Any failure is memory allocation related.
 */
int boot_mounts_add_mount(bool is_esp, MountInfo this_mount, BootMounts *mounts) {
    int ret = 0;
    size_t num = 0;
    size_t num_new = 0;

    /*
     * NB- since we explicitly allocated >= 1 element the error path
     *      if (mounts->efis) should never be false - but clang-tidy imagines 
     *      the num of new elems can be 0 for some (buggy) reason.
     */
    if (is_esp) {

        num = mounts->num_efis;
        num_new = mounts->num_efis + 1;
        ret = realloc_one(num_new, &mounts->num_efis, &mounts->efis);
        if (ret != 0) {
            goto exit;
        }

        if (mounts->efis) {
            mounts->efis[num] = this_mount;
        } else {
            ret = -1;
        }

    } else {

        num = mounts->num_xbootldrs;
        num_new = mounts->num_xbootldrs + 1;
        ret = realloc_one(num_new, &mounts->num_xbootldrs, &mounts->xbootldrs);
        if (ret != 0) {
            goto exit;
        }
        if (mounts->xbootldrs) {
            mounts->xbootldrs[num] = this_mount;
        } else {
            ret = -1;
        }
    }
exit:
    return ret;
}

