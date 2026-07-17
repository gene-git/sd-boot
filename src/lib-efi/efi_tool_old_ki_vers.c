// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Generate a list of ki_vers of any older installed of same efi tool package.
 *
 * - List of ki_vers excluding pkginfo->ki_vers.
 *   BLS layout by construction.
 *   list of dirs:
 *      <dol_boot>/>machine-id><package-name>-*
 *      <dol_boot>/loader/entries/<machine-id>-<package-name>-*.conf
 *      kern-vers = <pkg-name>-<pkg-vers>
 * Add any kern-vers != pkginfo->ki_vers to list.
 * Kernel packages are are handled by remove hook which is path trigger on
 *  usr/lib/modules/<kern-vers>vmlinuz. So when package is removed pacman
 *  tells us to remove it.
 */
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-config.h"
#include "sd-boot-efi.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"


static int extract_ki_vers(char *lead, Array_str *paths, size_t len_tail, PkgInfo *pkginfo) {

    if (paths->num_rows < 1) {
        return 0;
    }

    Array_str *ki_vers_old = &pkginfo->ki_vers_old;

    size_t len_lead = strlen(lead);
    size_t mark = 0;

    for (size_t i = 0; i < paths->num_rows; i++) {

        /*
         * remove extension like ".conf"
         */
        if (len_tail > 0) {
            mark = paths->row_len[i] - len_tail;
            paths->rows[i][mark] = '\0' ;
        }

        char * pkg_vers = paths->rows[i] + len_lead;
        char ki_vers[PATH_MAX] = {};
        size_t size = sizeof(ki_vers);

        int ret = 0;
        ret = snprintf(ki_vers, size, "%s-%s", pkginfo->pkg_name, pkg_vers);
        if (ret < 0 || ret > (int)size) {
            return -1;
        }
        if (strcmp(ki_vers, pkginfo->ki_vers) != 0) {
            if (!string_in_list(ki_vers, ki_vers_old->num_rows, ki_vers_old->rows)) {
                ret = array_str_add_string(ki_vers, ki_vers_old);
                if (ret != 0) {
                    return -1;
                }
            }
        }
    }

    return 0;
}


int efi_tool_ki_vers_old(SdBoot *conf, PkgInfo *pkginfo) {

    if (conf->tool_type != SDB_EFI_TOOL) {
        return 0;
    }

    char *dol_boot = nullptr;
    dol_boot = find_dollar_boot_bls(conf);
    if (!dol_boot) {
        return 0;
    }

    /*
     * Check for any old packages
     * For efi tools: ki_vers = <pkg-name>-<pkg-vers>
     * NB: must skip current pkginfo->ki_vers (obviously)
     * Append any older ki_vers to pkginfo->ki_vers_old
     */

    /*
     * - Files: $BOOT/loader/entries/<ENTRY-TOKEN>-<pkg-name>-<pkg-vers>.conf 
     */
    char lead[PATH_MAX] = {};
    char pattern[PATH_MAX] = {};
    size_t size_lead = sizeof(lead);
    size_t size_pattern = sizeof(pattern);
    Array_str paths_found = {};

    int ret = 0;
    ret = snprintf(lead, size_lead, "%s/loader/entries/%s-%s-", dol_boot, conf->entry_token, pkginfo->pkg_name);
    if (ret < 0 || ret > (int)size_lead) {
        ret = -1;
        goto exit;
    }

    const char *dot_conf = ".conf";
    size_t len_dot_conf = strlen(dot_conf);

    ret = snprintf(pattern, size_pattern, "%s*%s", lead, dot_conf);
    if (ret < 0 || ret > (int)size_pattern) {
        ret = -1;
        goto exit;
    }
    ret = file_list_glob(pattern, &paths_found);
    if (ret != 0) {
        ret = -1;
        goto exit;
    }

    ret = extract_ki_vers(lead, &paths_found, len_dot_conf, pkginfo);
    if (ret != 0) {
        ret = -1;
        goto exit;
    }

    array_str_free(&paths_found);

    /*
     * - Directories: $BOOT/<ENTRY-TOKEN>/<pkg-name>-<pkg-vers>
     */
    if (!conf->entry_token) {
        goto exit;
    }

    memset((void *)lead, 0, sizeof(lead));
    memset((void *)pattern, 0, sizeof(lead));

    ret = snprintf(lead, size_lead, "%s/%s/%s-", dol_boot, conf->entry_token, pkginfo->pkg_name);
    if (ret < 0 || ret > (int)size_lead) {
        ret = -1;
        goto exit;
    }
    ret = snprintf(pattern, size_pattern, "%s*", lead);
    if (ret < 0 || ret > (int)size_pattern) {
        ret = -1;
        goto exit;
    }
    ret = file_list_glob(pattern, &paths_found);
    if (ret != 0) {
        ret = -1;
        goto exit;
    }

    ret = extract_ki_vers(lead, &paths_found, 0, pkginfo);
    if (ret != 0) {
        ret = -1;
        goto exit;
    }

exit:
    if (dol_boot) {
        free((void *)dol_boot);
    }
    array_str_free(&paths_found);

    return ret;
}
