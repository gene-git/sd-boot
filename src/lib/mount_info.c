// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Information about a mount; Used for ESP and XBOOTLDR mounts.
 */
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

void mount_info_free(MountInfo *mount_info) {
    if (!mount_info) {
        return;
    }

    if (mount_info->device) {
        free((void *)mount_info->device);
        mount_info->device = nullptr;
    }

    if (mount_info->mount) {
        free((void *)mount_info->mount);
        mount_info->mount = nullptr;
    }
    mount_info->active = Unknown;
    mount_info->current = Unknown;
}

/*
 * Copy content from src to dst
 */
int mount_info_copy(MountInfo *src, MountInfo *dst) {
    int ret = 0;

    dst->active = src->active;
    dst->current = src->active;

    if (src->device) {
        dst->device = strdup(src->device);
        if (!src->device) {
            ret = -1;
            goto exit;
        }
    }

    if (src->mount) {
        dst->mount = strdup(src->mount);
        if (!src->mount) {
            ret = -1;
            goto exit;
        }
    }

exit:
    if (ret != 0) {
        msg(MSG_ERR, "  sd-boot copy mount info memory error\n");
    }
    return ret;
}
