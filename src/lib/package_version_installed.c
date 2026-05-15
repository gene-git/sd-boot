// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Invoke pacman to determine currently installed version of a package
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

int package_version_installed(const char *pkg, size_t len_vers, char *vers) {
    /*
     * Given package name lookup the installed version.
     * - return 0 on success
     * vers should be ROW_MAX bytes to be large enough for package version.
     */
    if (pkg == nullptr || vers == nullptr) {
        return -1;
    }

    vers[0] = '\0';

    int ret = 0;
    int child_ret = 0;
    char *output = nullptr;
    const char *argv[] = {"/usr/bin/pacman", "-Q", pkg, nullptr};
    const char *envp[] = {nullptr};

    ret = run_cmd_output((char **)argv, (char **)envp, &output, &child_ret) ;
    if (ret != 0) {
        msg(MSG_ERR, "  sd-boot: failed get efi from bootctl\n");
        return -1;
    }

    if (output != nullptr && output[0] == '\0') {
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

    bool found_pkg = false;
    char tmp[MAX_VAL_LEN+1] = {'\0'};
    char *tmp_ptr = nullptr;

    token = strtok_r(ptr, " ", &save_ptr);
    while (token != nullptr) {
        if (*token != '\0') {
            if (found_pkg) {
                strlcpy(tmp, token, len_vers);
                tmp_ptr = trim_string(tmp, len_vers);
                strlcpy(vers, tmp_ptr, len_vers);
                break;
            }
            if (strcmp(pkg, token) == 0) {
                found_pkg = true;
            }
        }
        token = strtok_r(nullptr, " ", &save_ptr);
    }
exit:
    if (output != nullptr) {
        free((void *) output);
    }
    return ret;
}
