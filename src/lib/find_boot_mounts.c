// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Identify the Mount Points for:
 *   EFI (where ESP is mounted)
 *   XBOOTLDR (if there is an XBOOTLDR partition)
 *
 * Parses output of systemd's bootctl.
 *
 * See https://www.kernel.org/pub/linux/utils/util-linux/v2.27/libblkid-docs/index.html
 */
#include <blkid/blkid.h>
#include <fcntl.h>
#include <mntent.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "sd-boot.h"

enum Constants {
    CHUNK = 32,
};


/*
 * Add one entry to list
 */
static int add_one_mount(char *fsname, Array_str *arr) {
    int ret = 0;
    size_t n_row = 0;

    n_row = arr->num_rows;

    if (arr->num_rows == 0) {
        ret = array_str_new(1, arr);
    } else {
        ret = array_str_resize(arr->num_rows + 1, arr);
    }
    if (ret != 0) {
        msg(MSG_ERR, "  sd-boot: memory alloc error\n");
        goto exit;
    }
    arr->rows[n_row] = strdup(fsname);

exit:
    return ret;
}

/**
 * Get partition GUID.
 * Caller must free memory returned.
 */
static char *get_partition_guid(const char *device) {
    int ret = 0;
    int fdes = 0; 
    const char *type_guid = nullptr;
    char *guid = nullptr;
    blkid_probe probe = nullptr;
    
    /* 
     * Try opening the path to mount
     * - requires root 
     */
    fdes = open(device, O_RDONLY | O_CLOEXEC);
    if (fdes < 0) {
        goto exit;
    }

    probe = blkid_new_probe();
    if (probe == nullptr) {
        fdes = 0;
        goto exit;
    }

    ret = blkid_probe_set_device(probe, fdes, 0, 0) ;
    if (ret < 0) {
        goto exit;
        //blkid_free_probe(probe);
        //close(fdes);
        //return nullptr;
    }

    /*
     * pull partition layout metadata from the parent disk
     */
    ret = blkid_probe_enable_partitions(probe, 1);
    if (ret < 0) {
        goto exit;
    };

    ret = blkid_probe_set_partitions_flags(probe, BLKID_PARTS_ENTRY_DETAILS);
    if (ret < 0) {
        goto exit;
    };

    /*
     * Gather results of probe.
     *  0 on success, 
     *  1 if nothing is detected, 
     * -2 if ambivalent result is detected and 
     * -1 on case of error.
     */
    ret = blkid_do_safeprobe(probe);
    if (ret == 1 || ret == -1) {
        goto exit;
    }

    /*
     * Get the partition entry type (guid)
     */
    ret = blkid_probe_lookup_value(probe, "PART_ENTRY_TYPE", &type_guid, nullptr);
    if (ret < 0) {
        goto exit;
    }

    if (type_guid != nullptr) {
        guid = strdup(type_guid);
    }

exit:
    if (probe != nullptr) {
        blkid_free_probe(probe);
    }
    if (fdes > 0) {
        close(fdes);
    }
    return guid;
}



int find_efi_xbootldr_mounts(Array_str *efi, Array_str *xbootldr) {
    /*
     * Locate the Mount Points for:
     *   EFI (where ESP is mounted)
     *   XBOOTLDR (if there is an XBOOTLDR partition)
     * Requires libblkid
     *
     * Allocates and returns an array of efi mount points and
     * array of xbootldr mount points.
     *
     * Call must free the arrays.
     */ 
    int ret = 0;
    FILE *fptr = nullptr;
    char *guid = nullptr;

    /*
     * Loop on mounts (other than devices) and check partition type.
     */
    fptr = setmntent("/proc/mounts", "r");
    if (fptr == nullptr) {
        perror(nullptr);
        msg(MSG_ERR, "  sd-boot: Error opening /proc/mounts\n");
        ret = -1;
        goto exit;
    }

    struct mntent *entry = {};
    const char *skip_dev = "/dev/";
    const size_t len_skip_dev = strlen(skip_dev);
    const char *ESP_GUID = "c12a7328-f81f-11d2-ba4b-00a0c93ec93b";
    const char *XBOOTLDR_GUID = "bc13c2ff-59e6-4262-a352-b275fd6f7172";

    while ((entry = getmntent(fptr)) != nullptr) {  // NOLINT(concurrency-mt-unsafe)
        /*
         * Skip non-relevant
         */
        if (strncmp(entry->mnt_fsname, skip_dev, len_skip_dev) != 0) {
            continue;
        }

        /*
         * Get partition entry type
         */
        guid = get_partition_guid(entry->mnt_fsname);
        if (!guid) {
            continue;
        }

        if (strcasecmp(guid, ESP_GUID) == 0) {
            ret = add_one_mount(entry->mnt_dir, efi);
            if (ret != 0) {
                msg(MSG_ERR, "  sd-boot: memory alloc error\n");
                goto exit;
            }

        } else if (strcasecmp(guid, XBOOTLDR_GUID) == 0) {
            ret = add_one_mount(entry->mnt_dir, xbootldr);
            if (ret != 0) {
                msg(MSG_ERR, "  sd-boot: memory alloc error\n");
                goto exit;
            }
        }
        free((void *)guid);
        guid = nullptr;
    }

exit:
    if (fptr != nullptr) {
        endmntent(fptr);
    }
    if (guid != nullptr) {
        free((void *)guid);
    }
    
    return ret;
}

