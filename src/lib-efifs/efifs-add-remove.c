// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Updates efi filesystem drivers by copying them from
 * /usr/lib/efifs-x64/ ==> "$efi"/EFI/systemd/drivers
 *
 */
#include <linux/limits.h>
#include <stdio.h>

#include "sd-boot-msg.h"
//#include "sd-boot-cmd.h"
#include "sd-boot-config.h"
#include "sd-boot-efifs.h"
#include "sd-boot-mounts.h"
#include "sd-boot-utils.h"


/*
 * Filesystem destination dir
 * - dest_dir is static array of size PATH_MAX
 * - load_config() guarantees conf->root exists and ends with '/'
 *   Either "/" or test root dir - also ends with "/"
 */
static int efifs_dest_dir(char *root, char *efi_dir, char *dest_dir) {

    /*
     * avoid double "//"
     */
    if (!efi_dir || !root || !dest_dir) {
        return -1;
    }

    char *efi_mount = efi_dir;
    if (*efi_dir == '/') {
        efi_mount = efi_dir + 1;
    }
    if (snprintf(dest_dir, PATH_MAX, "%s%s%s", root, efi_mount, "/EFI/systemd/drivers/") < 0) {
        perror(nullptr);
        return -1;
    }
    return 0;
}

/*
 * conf->oper must be one of:
 * - add or remove
 *
 * Locate efi
 * - add:
 *   - copy all the file system drivers from
 *     /usr/lib/efifs-x64/ => <efi>//EFI/systemd/drivers/
 *
 * - remove
 *   - remove fles from <efi>//EFI/systemd/drivers/
 *
 * Required Input Files:
 *  - none
 */
int efifs_add_remove(SdBoot *conf) {
    int ret = 0;

    if (!conf->efivars_available) {
        msg(MSG_ERR, "! sb-boot: warning cant find ESP mount point (chroot?)\n");
    }

    /*
     * locate efi
     */
    MountInfo efi_info = {};
    MountInfo xbootldr_info = {};
    if (find_boot_mounts_current(conf, &efi_info, &xbootldr_info) != 0) {
        ret = 1;
        goto exit;
    }

    if (efi_info.current != True) {
        msg(MSG_ERR, "! sd-boot: current EFI mount not found\n");
        ret = 1;
        goto exit;
    }

    char src[PATH_MAX] = {};
    char dst[PATH_MAX] = {};

    if (snprintf(src, PATH_MAX, "%s", "/usr/lib/efifs-x64/") < 0) {
        perror(nullptr);
        ret = 1;
        goto exit;
    }

    ret = efifs_dest_dir(conf->root, efi_info.mount, dst);
    if (ret != 0) {
        ret = 1;
        goto exit;
    }

    switch (conf->oper) {
        case KI_ADD:
            msg(MSG_NORMAL, "⦁ sd-boot: Copying efi filesystem drivers to %s\n", dst);
            /*
             * Will be cross device so use_fast = slow
             */
            ret = copy_directory_files(src, dst, false);
            if (ret != 0) {
                msg(MSG_ERR, "  ! sd-boot: error installing efi filesystem drivers\n");
                ret = 1;
                goto exit;
            }
            break;

        case KI_REMOVE:
            msg(MSG_NORMAL, "⦁ sd-boot: Removing efi filesystem drivers from %s\n", dst);

            ret = rm_rf(dst);
            if (ret != 0) {
                msg(MSG_ERR, "  ! sd-boot: error installing efi filesystem drivers\n");
                ret = 1;
                goto exit;
            }
            break;

        default:
            /*
             * cannot get here
             */
            break;
    }

exit:
    if (ret != 0 && !conf->efivars_available) {
        ret = 0;
    }
    mount_info_free(&efi_info);
    mount_info_free(&xbootldr_info);
    return ret;
}

