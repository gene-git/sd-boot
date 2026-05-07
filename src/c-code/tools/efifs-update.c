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

#include "sd-boot.h"

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

    if (argc < 2) {
        msg(MSG_ERR, "sd-boot: missing add or remove\n");
        ret = 1;
        goto exit;
    }
    /*
     * load config
     * - it sets verbosity level
     */
    SdBoot conf = {};
    if (load_config(&conf) != 0) {
        msg(MSG_ERR, "sd-boot: warning - no config file loaded - skipping\n");
    }

    int oper = BAD;

    oper = kernel_install_oper(argv[1]);
    if (oper == BAD) {
        msg(MSG_ERR, "sd-boot: missin add or remove but got %s\n", argv[1]);
        ret = 1;
        goto exit;
    }

    /*
     * locate efi
     */
    MountPoints mounts = {};
    if (find_efi_xbootldr_mounts(&mounts) != 0) {
        msg(MSG_ERR, "sd-boot: failed find EFI mount point\n");
        ret = 1;
        goto exit;
    }

    char src[PATH_MAX] = {'\0'};
    char dst[PATH_MAX] = {'\0'};

    if (snprintf(src, PATH_MAX, "%s", "/usr/lib/efifs-x64/") < 0) {
        perror(nullptr);
        ret = 1;
        goto exit;
    }

    if (snprintf(dst, PATH_MAX, "%s%s%s", conf.info.root, mounts.efi_dir, "/EFI/systemd/drivers/") < 0) {
        perror(nullptr);
        ret = 1;
        goto exit;
    }

    switch (oper) {
        case ADD:
            msg(MSG_NORMAL, "sd-boot: Copying efi filesystem drivers to %s\n", dst);

            char *const cmd_argv[] = {"/usr/bin/rsync", "--mkpath", "-a", src, dst, nullptr};
            char *const cmd_envp[] = {nullptr};
            int child_ret = 0;

            ret = run_cmd((char **)cmd_argv, (char **)cmd_envp, &child_ret);
            if (ret != 0 || child_ret != 0) {
                msg(MSG_ERR, "sd-boot: error installing efi filesystem drivers\n");
                ret = 1;
                goto exit;
            }
            break;

        case REMOVE:
            msg(MSG_NORMAL, "sd-boot: Removing efi filesystem drivers from %s\n", dst);

            ret = rm_rf(dst);
            if (ret != 0) {
                msg(MSG_ERR, "sd-boot: error installing efi filesystem drivers\n");
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
    clean_config(&conf);
    return ret;
}

