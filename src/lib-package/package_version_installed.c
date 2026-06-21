// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Invoke pacman to determine currently installed version of a package
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-cmd.h"
#include "sd-boot-config.h"
#include "sd-boot-package.h"
#include "sd-boot-utils.h"


int package_version_installed(SdBoot *conf, const char *pkg, size_t len_vers, char *vers) {
    /*
     * Given package name lookup the installed version.
     * - return 0 on success
     * vers should be large enough for package version.
     */
    if (!pkg || !vers) {
        return -1;
    }
    Array_str *env_base = &conf->env_base;

    vers[0] = '\0';

    int ret = 0;
    int child_ret = 0;
    char *output = nullptr;
    const char *argv[] = {"/usr/bin/pacman", "-Q", pkg, nullptr};

    ret = run_cmd_output((char **)argv, env_base->rows, &output, &child_ret) ;
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: failed get package info from pacman\n");
        return -1;
    }

    if (output && output[0] == '\0') {
        ret = -1;
        goto exit;
    }

    /*
     * extract the version returned
     * - string returned is: <pkg> <vers>
     * - after see package then next token to come is the version
    */
    char *token = nullptr;
    char *ptr = output;
    char *save_ptr = nullptr;

    char tmp[KV_MAX_VAL_LEN+1] = {};
    char *tmp_ptr = nullptr;

    token = strtok_r(ptr, " ", &save_ptr);
    while (token) {
        if (token[0] != '\0') {
            if (strcmp(pkg, token) == 0) {
                token = strtok_r(nullptr, " ", &save_ptr);
                if (!token) {
                    goto exit;
                }
                strlcpy(tmp, token, KV_MAX_VAL_LEN);
                tmp_ptr = trim_string(tmp, len_vers);
                strlcpy(vers, tmp_ptr, len_vers);
                break;
            }
        }
        token = strtok_r(nullptr, " ", &save_ptr);
    }
exit:
    if (output) {
        free((void *) output);
    }
    return ret;
}
