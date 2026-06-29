// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read Package Versions File
 * - /var/lib/sd-boot/<pkgbase>.version
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-msg.h"
#include "sd-boot-package.h"



/*
 * New Current Version:
 *
 * - kernels track kern-vers (pkginfo->ki_vers) taken from /usr/lib/modules/<kern-vers>
 * - efi-tools track the package version.
 */
static char *get_current_version(SdBoot *conf, PkgInfo *pkginfo) {
    char *new_curr = nullptr;

    switch (conf->tool_type) {
        case SDB_KERNEL:
            if (!pkginfo->ki_vers) {
                msg(MSG_ERR, "!  sd-boot: version update missing kernel vers\n"); 
                return nullptr;
            }
            new_curr = strdup(pkginfo->ki_vers);
            break;

        case SDB_EFI_TOOL:
            if (!pkginfo->pkg_vers) {
                msg(MSG_ERR, "!  sd-boot: version update missing pkg vers\n"); 
                return nullptr;
            }
            new_curr = strdup(pkginfo->pkg_vers); 
            break;

        default:
            break;
    }

    return new_curr;
}

/*
 * Update package versions and save 
 *
 * There are 2 kinds of "versions" used.
 * For kernels version is taken from /usr/lib/modules/<kern-vers>
 *  - current version is provided in pkginfo->vers
 *
 * For efi-tools the version is the package version
 *  - current version is provided in pkginfo->pkg_vers
 *
 * The following 2 fields are updated here:
 * - pkginfo->vers_curr 
 * - pkginfo->vers_prev
 *
 * then the udpated valaues are written to file /var/lib/sdb-boot/<name>.versions
 * If available, current values of vers_curr/vers_prev non-null
 * - these are read in initialize_tool()
 */
int update_package_versions(SdBoot *conf, PkgInfo *pkginfo) {
    int ret = 0;
    char *new_curr = nullptr;

    /*
     * New Current Version:
     *
     * - kernels track kern-vers (pkginfo->ki_vers) taken from /usr/lib/modules/<kern-vers>
     * - efi-tools track the package version.
     */
    new_curr = get_current_version(conf, pkginfo);

    if (!new_curr) {
        ret = -1;
        goto exit;
    }

    /*
     * previous
     */
    bool updated = false;

    if (pkginfo->vers_curr) {

        if (strcmp(pkginfo->vers_curr, new_curr) != 0) {

            if (pkginfo->vers_prev) {
                free((void *)pkginfo->vers_prev);
                pkginfo->vers_prev = nullptr;
            }

            pkginfo->vers_prev = pkginfo->vers_curr;
            pkginfo->vers_curr = new_curr;
            new_curr = nullptr;
        }

    } else {
        updated = true;
        pkginfo->vers_curr = new_curr;
        new_curr = nullptr;
    }

    if (updated) {
        ret = write_package_version_file(conf, pkginfo);
        if (ret < 0) {
            goto exit;
        }
    }

    /*
     * sanity check
     */
    if (!pkginfo->vers_curr) {
        msg(MSG_ERR, "  ! sd-boot: add missing curr ki version\n");
        ret = -1;
    }

exit:
    if (new_curr) {
        free((void *)new_curr);
    }
    return ret;
}


