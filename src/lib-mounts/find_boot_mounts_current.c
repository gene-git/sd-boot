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

#include "sd-boot-msg.h"
#include "sd-boot-cmd.h"
#include "sd-boot-config.h"
#include "sd-boot-mounts.h"
#include "sd-boot-utils.h"

/*
 * If token matches then strdup() it and return allocated string
 * Returns 0 on success otherwise -1 for error.
 */
static int token_match_key(char *token, char **save_ptr, const char *key, char **match_p) {
    int ret = 0;

    if (strncmp(key, (const char *)token, strlen(key)) == 0) {
        token = strtok_r(nullptr, " ", save_ptr);
        if (token) {
            *match_p = strdup(token);
            if (*match_p == nullptr) {
                ret = -1;
                goto exit;
            }
        }
    }

exit:
    return ret;
}

static int extract_efi_xbootldr(char *buf, char **efi_p, char **xbootldr_p) {
    /*
     * extract mount points:
     * - efi from string: "ESP: <efi> ..."
     * - xbootldr from string: "XBOOTLDR: <xbootldr> ..."
     * NB this modifies content of buf
     *
     * Only extract the first instance.
     */
    if (!buf || !efi_p || !xbootldr_p) {
        return -1;
    }
    char *token = nullptr;
    char *save_ptr = nullptr;

    const char *efi_key = "ESP:";
    const char *xbt_key = "XBOOTLDR:";

    token = strtok_r(buf, " ", &save_ptr);
    if (!token) {
        return 0;
    }
    
    int ret = 0;

    /*
     * check if token matches a key
     */
    if (*efi_p == nullptr) {
        ret = token_match_key(token, &save_ptr, efi_key, efi_p);
        if (ret != 0) {
            goto exit;
        }
    }

    if (*xbootldr_p == nullptr) {
        ret = token_match_key(token, &save_ptr, xbt_key, xbootldr_p);
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    return ret;
}

/*
 * Locate the ESP mount point (efi) for current boot
 * With multiple disks, there can be multiple ESPs some/all may be mounted.
 * Instead we rely on bootctl to identofy  the relevant EFI for us.
 * Parses output of systemd's bootctl.
 *
 * On success efi->mount is set and efi->current is True
 * Likewise for xbootldr.
 */ 

int find_boot_mounts_current(SdBoot *conf, MountInfo *efi, MountInfo *xbootldr) {
    int ret = 0;
    int child_ret = 0;
    Array_str *env_base = &conf->env_base;;

    if (!efi || !xbootldr) {
        return 1;
    }

    mount_info_free(efi);
    mount_info_free(xbootldr);

    char *output = nullptr;
    char *ptr = nullptr;
    char *line = nullptr;
    char *line_tmp = nullptr;
    char *argv[] = {"/usr/bin/bootctl", nullptr};

    /*
     * Run cmd
     */
    ret = run_cmd_output(argv, env_base->rows, &output, &child_ret) ;
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: failed get efi from bootctl\n");
        return -1;
    }

    if (!output || output[0] == 0) {
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
    while ((line = get_one_line(&ptr))) {

        line_tmp = strdup(line);
        if (!line_tmp) {
            msg(MSG_ERR, "  ! sd-boot: memory allocation fail\n");
            ret = -1;
            goto exit;
        }
        line_len = (size_t) strlen(line_tmp);
        cleaned = trim_string(line_tmp, line_len);

        extract_efi_xbootldr(cleaned, &efi->mount, &xbootldr->mount);

        free((void *)line_tmp);
        line_tmp = nullptr;

        if (efi->mount && xbootldr->mount) {
            break;
        }
    }

    if (efi->mount) {
        efi->current = True;
    } else {
        msg(MSG_ERR, "  sd-boot failed to identify current ESP mount\n");
        ret = -1;
    }

    if (xbootldr->mount) {
        xbootldr->current = True;
    }

exit:
    if (output) {
        free((void *)output);
    }
    if (line_tmp) {
        free((void *) line_tmp);
    }
    
    return ret;
}
