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
#include "sd-boot.h"

enum PackageConst {
    PKG_LEN = KV_MAX_VAL_LEN + 1,
};

/*
 * Extract the version string from 'pacman -Q' output
 * - output takes the form: <pkg> <vers>
 * - So look for package name, then next token is the version
 */
static int extract_version(char *output, char *pkg_name, char **pkg_vers_p) {
    int ret = 0;
    char *token = nullptr;
    char *ptr = output;
    char *save_ptr = nullptr;

    char tmp[PKG_LEN] = {};
    char *tmp_ptr = nullptr;

    *pkg_vers_p = nullptr;

    token = strtok_r(ptr, " ", &save_ptr);
    while (token) {

        if (token[0] != '\0') {
            /*
             * Find package name and the next token is the version
             */
            if (strcmp(pkg_name, token) == 0) {
                token = strtok_r(nullptr, " ", &save_ptr);
                if (!token) {
                    goto exit;
                }

                if (strlcpy(tmp, token, PKG_LEN) >= PKG_LEN) {
                    ret = -1;
                    goto exit;
                }
                tmp_ptr = trim_string(tmp, PKG_LEN);

                *pkg_vers_p = strdup(tmp_ptr);
                if (*pkg_vers_p == nullptr) {
                    ret = -1;
                    goto exit;
                }
                break;
            }
        }

        token = strtok_r(nullptr, " ", &save_ptr);
    }
exit:
    return ret;
}


/*
 * Given package name lookup the installed version.
 *
 * Input:
 *  pkginfo->pkg_name
 *
 * Output:
 *  pkginfo->pkg_vers
 *
 * Return:
 *   0 on success
 */
int package_version_installed(SdBoot *conf, PkgInfo *pkginfo) {
    int ret = 0;

    if (!pkginfo || !pkginfo->pkg_name) {
        return -1;
    }
    Array_str *env_base = &conf->env_base;

    int child_ret = 0;
    char *output = nullptr;
    const char *argv[] = {"/usr/bin/pacman", "-Q", pkginfo->pkg_name, nullptr};

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
     */
    ret = extract_version(output, pkginfo->pkg_name, &pkginfo->pkg_vers);
    if (ret != 0) {
        goto exit;
    }

exit:
    if (output) {
        free((void *) output);
    }
    return ret;
}
