// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Locate the Mount Points for:
 *   EFI (where ESP is mounted)
 *   XBOOTLDR (if there is an XBOOTLDR partition)
 *
 * Parses output of systemd's bootctl.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

enum BufSize {
    BUF_SIZE = 1024,
};

static void extract_efi_xbootldr(char *buf, size_t size, MountPoints *mounts) {
    /*
     * extract mount points:
     * - efi from string: "ESP: <efi> ..."
     * - xbootldr from string: "XBOOTLDR: <xbootldr> ..."
     * NB this modifies content of buf
     *
     * Only extract the first instance.
     */
    if (buf == nullptr || mounts == nullptr) {
        return;
    }
    char *token = nullptr;
    char *save_ptr = nullptr;
    size_t length = 0;

    const char *efi_key = "ESP:";
    const char *xbt_key = "XBOOTLDR:";

    token = strtok_r(buf, " ", &save_ptr);
    if (token != nullptr) {
        /*
         * check if token matches a key
         */
        if (strncmp(efi_key, (const char *)token, strlen(efi_key)) == 0) {
            if (mounts->efi_dir[0] == '\0') {
                token = strtok_r(nullptr, " ", &save_ptr);
                if (token != nullptr) {
                    length = strnlen(token, size);
                    strncpy(mounts->efi_dir, token, length);
                }
            }

        } else if (strncmp(xbt_key, (const char *)token, strlen(xbt_key)) == 0) {
            if (mounts->xbootldr_dir[0] == '\0') {
                token = strtok_r(nullptr, " ", &save_ptr);
                if (token != nullptr) {
                    length = strnlen(token, size);
                    strncpy(mounts->xbootldr_dir, token, length);
                }
            }
        }
        // token = strtok_r(nullptr, " ", &save_ptr);
    }
}


int find_efi_current_boot(MountPoints *mounts) {
    /*
     * Locate the ESP mount point (efi) for current boot
     * With multiple disks, there can be multiple ESPs some/all may be mounted.
     * Instead we rely on bootctl to identofy  the relevant EFI for us.
     * Parses output of systemd's bootctl.
     */ 
    int ret = 0;
    int child_ret = 0;

    if (mounts == nullptr) {
        return 1;
    }
    mounts->efi_dir[0] = '\0';
    mounts->xbootldr_dir[0] = '\0';

    char *output = nullptr;
    char *ptr = nullptr;
    char *line = nullptr;
    char *line_tmp = nullptr;
    char *argv[] = {"/usr/bin/bootctl", nullptr};
    char *envp[] = {nullptr};

    ret = run_cmd_output(argv, envp, &output, &child_ret) ;
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: failed get efi from bootctl\n");
        return -1;
    }

    if (output == nullptr || output[0] == 0) {
        goto exit;
    }

    /*
     * Break bootctl output into lines.
     */
    char *cleaned = nullptr;
    size_t line_len = 0;

    /*
     * nb: dont change line inside loop.
     * - trim_string moves chars around so make copy of line
     */
    ptr = output;
    while ((line = get_one_line(&ptr)) != nullptr) {

        line_tmp = strdup(line);
        if (line_tmp == nullptr) {
            msg(MSG_ERR, "  ! sd-boot: memory allocation fail\n");
            goto exit;
        }
        line_len = (size_t) strlen(line_tmp);
        cleaned = trim_string(line_tmp, line_len);
        line_len = strlen(cleaned);

        extract_efi_xbootldr(cleaned, line_len, mounts);

        free((void *)line_tmp);
        line_tmp = nullptr;

        if (mounts->efi_dir[0] != '\0' && mounts->xbootldr_dir[0] != '\0') {
            break;
        }
    }

exit:
    if (output != nullptr) {
        free((void *)output);
    }
    if (line_tmp != nullptr) {
        free((void *) line_tmp);
    }
    
    return ret;
}
