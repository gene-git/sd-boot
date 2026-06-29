// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-mounts.h
 */
#ifndef SD_BOOT_MOUNTS_H
#define SD_BOOT_MOUNTS_H

#include <libmount/libmount.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#include "sd-boot-config.h"

/*
 * Mount Points : efi and xbootldr partitions.
 *
 * With mulitple ESP partitions (multiple disks), 
 * the currently mounted ESP mount may differ from that used to 'boot'
 * and saved to nvram by the boot loader we show both views. 
 *
 * We report these using:
 * - current = for currently mounted 
 * - active = as reported by boot loader and saved to nvram.
 *
 * For currently mounted determination we use bootctl.
 * 
 * - efi - visible to all users.
 * - xbootldr - requires root 
 *
 * Provided by udev/libmount:
 * - all ESP and XBOOTLDR mount points.
 */
typedef enum TriState: signed char {
    Unknown = -1,
    False = 0,
    True = 1,
} TriState;

typedef struct MountInfo {
    char *device;
    char *mount;
    TriState active;
    TriState current;
} MountInfo;

typedef struct BootMounts {
    size_t num_efis;
    size_t num_xbootldrs;

    MountInfo *efis;
    MountInfo *xbootldrs;
} BootMounts;

/*
 * Function declarations
 */

/*
 * lib
 */
void mount_info_free(MountInfo *mount_info);
int mount_info_copy(MountInfo *src, MountInfo *dst);

int boot_mounts_alloc(size_t num_efis, size_t num_xbootldrs, BootMounts *boot_mounts);
int boot_mounts_add_mount(bool is_esp, MountInfo this_mount, BootMounts *mounts);
void boot_mounts_free(BootMounts *boot_mounts);
bool mount_is_block_device(struct libmnt_fs *entry);
int find_boot_mounts(SdBoot *conf, BootMounts *mounts);
int find_boot_mounts_current(SdBoot *conf, MountInfo *efi, MountInfo *xbootldr);
int boot_mounts_mark_current(SdBoot *conf, BootMounts *boot_mounts);

#endif
