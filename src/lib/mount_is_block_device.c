// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
  * Given mount table entry, determine if it is a physical block device
  */ 
#include <libmount/libmount.h>
#include <stdbool.h>
#include <string.h>


bool mount_is_block_device(struct libmnt_fs *entry) {
    if (!entry) {
        return false;
    }
    //const char *target = mnt_fs_get_target(entry);
    // const char *fstype = mnt_fs_get_fstype(entry);
    const char *fs_root = mnt_fs_get_root(entry);

    /*
     * /proc, /run, /tmp etc
     */
    if (mnt_fs_is_pseudofs(entry)) {
        return false;
    }

    /*
     * bind mount
     */
    if (fs_root && strcmp(fs_root, "/") != 0) {
        return false;
    }

    return true;
}

