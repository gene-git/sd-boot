// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Locate the Mount Points for:
 *   EFI (where ESP is mounted)
 *   XBOOTLDR (if there is an XBOOTLDR partition)
 *
 * Parses output of systemd's bootctl.
 */
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

enum BufSize {
    BUF_SIZE = 1024,
};

static int extract_efi_xbootldr(char *buf, size_t size, MountPoints *mounts) {
    /*
     * extract mount points:
     * - efi from string: "ESP: <efi> ..."
     * - xbootldr from string: "XBOOTLDR: <xbootldr> ..."
     * NB this modifies content of buf
     */
    if (buf == nullptr || mounts == nullptr) {
        return -1;
    }

    mounts->efi_dir[0] = '\0';
    mounts->xbootldr_dir[0] = '\0';

    char *token = nullptr;
    char *save_ptr = nullptr;

    /*
     * efi - always exists
     */
    const char *key_efi = "ESP:";
    const size_t len_key_efi = strnlen(key_efi, 16) ;
    bool seen_efi = false;
    bool saved_efi = false;

    /*
     * xbootldr - may not exist.
     */
    const char *key_xbootldr = "XBOOTLDR:";
    const size_t len_key_xbootldr = strnlen(key_xbootldr, 16) ;
    bool seen_xbootldr = false;
    bool saved_xbootldr = false;

    size_t length = 0;
    token = strtok_r(buf, " ", &save_ptr);
    while (token != nullptr) {
        /*
         * save token when prev one matched key (seen_key)
         */
        if (seen_efi && !saved_efi) {
            length = strnlen(token, size);
            strncpy(mounts->efi_dir, token, length);
            saved_efi = true;
        }
        if (seen_xbootldr && !saved_xbootldr) {
            length = strnlen(token, size);
            strncpy(mounts->xbootldr_dir, token, length);
            saved_xbootldr = true;
        }

        if (saved_efi && saved_xbootldr) {
            break;
        }

        /*
         * check if token matches a key
         */
        if (!seen_efi && strncmp(key_efi, token, len_key_efi) == 0) {
            seen_efi = true;
        }
        if (!seen_xbootldr && strncmp(key_xbootldr, token, len_key_xbootldr) == 0) {
            seen_xbootldr = true;
        }
        token = strtok_r(nullptr, " ", &save_ptr);
    }

    return 0;
}


int find_efi_xbootldr_mounts(MountPoints *mounts) {
    /*
     * Locate the Mount Points for:
     *   EFI (where ESP is mounted)
     *   XBOOTLDR (if there is an XBOOTLDR partition)
     *
     * Parses output of systemd's bootctl.
     */ 
    char buf[PATH_MAX] = {'\0'};
    int ret = 0;
    int child_ret = 0;
    size_t size = sizeof(buf);

    if (mounts == nullptr) {
        return 1;
    }
    mounts->efi_dir[0] = '\0';
    mounts->xbootldr_dir[0] = '\0';

    char *output = nullptr;
    char *line_tmp = nullptr;
    char *argv[] = {"/usr/bin/bootctl", nullptr};
    char *envp[] = {nullptr};

    ret = run_cmd_output(argv, envp, &output, &child_ret) ;
    if (ret != 0) {
        msg(MSG_ERR, "  sd-boot: failed get efi from bootctl\n");
        return -1;
    }

    if (output == nullptr || output[0] == 0) {
        goto exit;
    }

    /*
     * Break bootctl output into lines.
     * - the line after "matcher" has the ESP mount point
     */
    const char *matcher = "Available Boot Loaders on ESP:";
    const size_t len_matcher = strlen(matcher);
    bool matched = false;
    char *cleaned = nullptr;
    char *ptr = output;
    char *line = nullptr;
    size_t line_len = 0;
    size_t line_tmp_size = PATH_MAX * sizeof(char);

    line_tmp = (char *) calloc(line_tmp_size, sizeof(char));

    /*
     * nb: dont change line inside loop.
     * - trim_string moves chars around so make copy of line
     */
    bool found = false;
    while ((line = get_one_line(&ptr)) != nullptr) {
        if (!found) {

            line_len = (size_t) strlen(line);
            if (line_len > line_tmp_size) {
                line_tmp_size = line_len;
                char *tmp_ptr = (char *)realloc(line_tmp, line_tmp_size + sizeof(char));
                if (tmp_ptr == nullptr) {
                    perror(nullptr);
                    goto exit;
                }
                line_tmp = tmp_ptr;
            }
            strncpy(line_tmp, line, line_len+1);

            cleaned = trim_string(line_tmp, line_tmp_size);
            if (matched) {
                found = true;
                strncpy(buf, cleaned, size-1);
            }
            if (strncmp(cleaned, matcher, len_matcher) == 0) {
                matched = true;
            }
        }
    }

    if (buf[0] != '\0') {
        line = strdup(buf);
        line_len = strlen(buf);

        ret = extract_efi_xbootldr(line, line_len, mounts);
        free(line);
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
