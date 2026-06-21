// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Updates the efi filesystem drivers by copying from
 * /usr/lib/efifs-x64/ ==> "$efi"/EFI/systemd/drivers
 *
 * Supports SDB_DEV_TEST which sets BOOT_ROOT to the testing tree.
 */
#include <linux/limits.h>
#include <stdio.h>

#include "sd-boot-msg.h"
#include "sd-boot-cmd.h"
#include "sd-boot-config.h"
#include "sd-boot-mounts.h"
#include "sd-boot-utils.h"

int main(int argc, char *argv[]) {
    /*
     * Takes one argument:
     * - add or remove
     *
     * Locate efi
     * - add:
     *   - copy all the file system drivers from
     *   /usr/lib/efifs-x64/ => <efi>//EFI/systemd/drivers/
     *
     * - remove
     *   - remove fles from <efi>//EFI/systemd/drivers/
     *
     * Triggers:
     *  - none
     * 
     * Required Input Files:
     *  - none
     */
    int ret = 0;
    SdBoot conf = {};

    if (argc < 2) {
        msg(MSG_ERR, "! sd-boot: missing add or remove\n");
        ret = 1;
        goto exit;
    }

    /*
     * load config
     * - it sets verbosity level
     */
    if (load_config(&conf) != 0) {
        msg(MSG_ERR, "! sd-boot: warning - no config file loaded - skipping\n");
    }

    if (!conf.efivars_available) {
        msg(MSG_ERR, "! sb-boot: warning cant find ESP mount point (in chroot?)\n");
    }

    KernelInstallOper oper = KI_BAD;

    oper = kernel_install_oper((const char *)argv[1]);
    if (oper == KI_BAD) {
        msg(MSG_ERR, "! sd-boot: missin add or remove but got %s\n", argv[1]);
        ret = 1;
        goto exit;
    }

    /*
     * locate efi
     */
    MountInfo efi_info = {};
    MountInfo xbootldr_info = {};
    if (find_boot_mounts_current(&conf, &efi_info, &xbootldr_info) != 0) {
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

    if (snprintf(dst, PATH_MAX, "%s%s%s", conf.root, efi_info.mount, "/EFI/systemd/drivers/") < 0) {
        perror(nullptr);
        ret = 1;
        goto exit;
    }

    switch (oper) {
        case KI_ADD:
            msg(MSG_NORMAL, "⦁ sd-boot: Copying efi filesystem drivers to %s\n", dst);

            char *const cmd_argv[] = {"/usr/bin/rsync", "--mkpath", "-a", src, dst, nullptr};
            int child_ret = 0;

            ret = run_cmd((char **)cmd_argv, conf.env_base.rows, &child_ret);
            if (ret != 0 || child_ret != 0) {
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
    if (ret != 0 && !conf.efivars_available) {
        ret = 0;
    }
    mount_info_free(&efi_info);
    mount_info_free(&xbootldr_info);
    config_clean(&conf);
    return ret;
}

