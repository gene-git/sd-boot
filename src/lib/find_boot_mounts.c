// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Identify ESP and XBOOTLDR partitions
 * Flag those that are used in current boot.
 *
 * Approach:
 *
 * Identify the Active ESP directly via LoaderDevicePartUUID.
 * Identify the parent disk of that active ESP (e.g., nvme0n1).
 * Active Flag Logic:
 *
 * ESP: 
 * - active if its partition UUID matches LoaderDevicePartUUID exactly.
 *
 * XBOOTLDR: 
 * - active if on same physical disk as the active ESP. 
 *  (Per Freedesktop Boot Loader Specification, a companion XBOOTLDR partition must 
 *   share the same physical drive structure as the booting ESP to be parsed by systemd-boot).
 * Cross-Disk ESP Fallback: 
 * - If ESP on a different disk but is actively mounted under /efi or /boot, 
 *   - catch it via real-time mount paths. ￼
 */
#include <libmount/libmount.h> 
#include <libudev.h>
#include <stdbool.h> 
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h> 
#include <sys/types.h> 

#include "sd-boot.h"

enum MiscConstants { BUF_LEN = 128, };

/**
 * Get active boot partition UUID from efivars.
 */
char *get_active_esp_uuid(void) {
    char efi_var_path[BUF_LEN] = {};
    char *uuid_str = nullptr;
    const char *efi_vars = "/sys/firmware/efi/efivars/";
    const char *loader_device_uuid = "LoaderDevicePartUUID-4a67b082-0a4c-41cf-b6c7-440b29bb8c4f";

    if (snprintf(efi_var_path, sizeof(efi_var_path), "%s%s", efi_vars, loader_device_uuid) < 0) {
        goto exit;
    }

    uuid_str = read_efi_var_string(efi_var_path);

exit:
    return uuid_str;
}

/*
 * Obvious dirs to skip which are not
 * ESP or XBOOTLDR mounts
 */
static bool skip_mount(const char *path) {
    const char *names[] = {
        "/dev/",
        "/dev",
        "/sys/",
        "/sys",
        "/proc/",
        "/proc",
        "/run/",
        "/run",
        "/tmp/",
        "/tmp",
    };
    const size_t num_names = sizeof(names) / sizeof(names[0]);

    if (path == nullptr) {
        return true;
    }

    if (strcmp(path, "/") == 0) {
        return true;
    }

    for (size_t i = 0 ; i < num_names; i++) {
        const char *name = names[i];
        if (strncmp(path, name, strlen(name)) == 0) {
            return true;
        }
     }

    return false;
}

/**
 * Use active partition UUID to resolve its parent physical disk device number (dev_t).
 * Return: The dev_t device identifier of the parent disk, or 0 on failure.
 */
static dev_t active_parent_dev_t(struct udev *udev, struct libmnt_table *mount_table, const char *active_uuid) {
    dev_t dev = 0;
    dev_t parent_dev = 0;
    struct libmnt_iter *iter = nullptr;
    struct libmnt_fs *mount_entry = nullptr;
    struct udev_device *device = nullptr;
    struct udev_device *parent_device = nullptr;

    if (!udev || !mount_table || !active_uuid) { 
        return dev;
    }

    iter = mnt_new_iter(MNT_ITER_FORWARD);
    if (!iter) {
        goto exit;
    }
    while (mnt_table_next_fs(mount_table, iter, &mount_entry) == 0) {
        const char *src = mnt_fs_get_srcpath(mount_entry);
        const char *target = mnt_fs_get_target(mount_entry);

        if (skip_mount(target)) {
            continue;
        }

        struct stat stat_buf = {};
        if (stat(src, &stat_buf) < 0) {
            continue;
        }

        device = udev_device_new_from_devnum(udev, 'b', stat_buf.st_rdev);
        if (!device) {
            continue;
        }

        const char *p_guid = udev_device_get_property_value(device, "ID_PART_ENTRY_UUID");
        if (p_guid && strcasecmp(p_guid, active_uuid) == 0) {
            parent_device = udev_device_get_parent_with_subsystem_devtype(device, "block", "disk");
            if (parent_device) {
                parent_dev = udev_device_get_devnum(parent_device);
            }
            udev_device_unref(device);
            device = nullptr;
            break;
        }
        udev_device_unref(device);
        device = nullptr;
    }

exit:
    if (iter) {
        mnt_free_iter(iter);
    }
    if (device) {
        udev_device_unref(device);
    }
    return parent_dev;
}

/*
 * Is partition active
 */
static bool is_active(bool is_esp, bool is_xboot, const char *guid,
                      const char *active_uuid, dev_t parent_num, dev_t active_num) {

    /*
     * ESP match from the motherboard NVRAM
     */
    if (is_esp && active_uuid && strcasecmp(guid, active_uuid) == 0) {
        return true;
    }

    if (is_xboot && active_num != 0 && parent_num == active_num) {
        return true;
    }

    return false;
}

/**
 * @brief Evaluates an extracted partition device node against system criteria.
 */
static void process_partition_node(struct udev *udev, struct libmnt_fs *mount_entry,
                                    const char *active_uuid, dev_t active_esp_dev_t, BootMounts *boot_mounts) {
    struct udev_device *dev = nullptr;
    struct udev_device *parent = nullptr;
    const char *GPT_ESP_TYPE = "c12a7328-f81f-11d2-ba4b-00a0c93ec93b";
    const char *GPT_XBOOTLDR_TYPE = "bc13c2ff-59e6-4262-a352-b275fd6f7172";
    struct stat stat_buf = {};

    const char *src = mnt_fs_get_srcpath(mount_entry);

    const char *target = mnt_fs_get_target(mount_entry);
    if (!src || skip_mount(target)) {
        goto exit;
    }

    if (stat(src, &stat_buf) < 0) {
        goto exit;
    }

    dev = udev_device_new_from_devnum(udev, 'b', stat_buf.st_rdev);
    if (!dev) {
        goto exit;
    }

    const char *p_type = udev_device_get_property_value(dev, "ID_PART_ENTRY_TYPE");
    const char *p_guid = udev_device_get_property_value(dev, "ID_PART_ENTRY_UUID");

    if (!p_type || !p_guid) {
        goto exit;
    }

    bool is_esp = (bool)(strcasecmp(p_type, GPT_ESP_TYPE) == 0);
    bool is_xboot = (bool)(strcasecmp(p_type, GPT_XBOOTLDR_TYPE) == 0);

    if (is_esp || is_xboot) {
        dev_t parent_devnum = 0;
        parent = udev_device_get_parent_with_subsystem_devtype(dev, "block", "disk");
        if (parent) {
            parent_devnum = udev_device_get_devnum(parent);
        }

        bool active = is_active(is_esp, is_xboot, p_guid, active_uuid, parent_devnum, active_esp_dev_t);

        MountInfo new_mount = {};
        new_mount.device = strdup(src);
        new_mount.mount = strdup(target ? target : " - ");
        new_mount.active = active;

        if (boot_mounts_add_mount(is_esp, new_mount, boot_mounts) != 0){
            free((void *)new_mount.device);
            free((void *)new_mount.mount);
        }
    }

exit:
    if (dev) {
        udev_device_unref(dev);
    }
}


static int initialize(struct libmnt_table **mount_table_p, struct udev **udev_p) {
    int ret = 0;
    struct libmnt_table *mount_table = nullptr;
    struct udev *udev = nullptr;

    /*
    mounts->num_efis = 0;
    mounts->num_xbootldrs = 0;
    mounts->efis = nullptr;
    mounts->xbootldrs = nullptr;
    */

    /*
     * - Load active mounts into libmount table.
     * - parse /proc/mounts
     */
    mount_table = mnt_new_table();
    if (!mount_table) {
        ret = -1;
        goto exit;
    }
    *mount_table_p = mount_table;

    if (mnt_table_parse_mtab(mount_table, nullptr) < 0) {
        ret = -1;

        goto exit;
    }

    udev = udev_new();
    if (!udev) {
        ret = -1;
        goto exit;
    }
    *udev_p = udev;
exit:
    if (ret != 0 && mount_table) {
        mnt_free_table(mount_table);
    }
    return ret;
}

/**
 * Check system block topologies, identifying every ESP and XBOOTLDR partition.
 * - check whats mounted using libmount (and /proc/mounts)
 * returns 
 *  - 0 on a success else -1
 */
int find_boot_mounts(BootMounts *boot_mounts) {
    int ret = 0;
    struct libmnt_table *mount_table = nullptr;
    struct udev *udev = nullptr;
    char *active_esp_uuid = nullptr;
    struct libmnt_iter *iter = nullptr;
    struct libmnt_fs *mount_entry = nullptr;

    if (!boot_mounts) {
        return -1;
    }

    ret = initialize(&mount_table, &udev);
    if (ret != 0) {
        goto exit;
    }

    active_esp_uuid = get_active_esp_uuid();
    dev_t active_esp_dev_t = 0;
    active_esp_dev_t = active_parent_dev_t(udev, mount_table, active_esp_uuid);

    /*
     * walk the mount table (/proc/mounts)
     */
    iter = mnt_new_iter(MNT_ITER_FORWARD);
    if (!iter) {
        goto exit;
    }
    while (mnt_table_next_fs(mount_table, iter, &mount_entry) == 0) {
        process_partition_node(udev, mount_entry, active_esp_uuid, active_esp_dev_t, boot_mounts);
    }


exit:
    if (iter) {
        mnt_free_iter(iter);
    }

    if (mount_table) {
        mnt_free_table(mount_table);
    }

    if (udev) {
        udev_unref(udev);
    }

    if (active_esp_uuid) {
        free((void *)active_esp_uuid);
    }

    return ret;
}

