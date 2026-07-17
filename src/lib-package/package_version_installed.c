// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Use libalpm to determine currently installed version of a package
 */

#include "sd-boot-alpm.h"
#include "sd-boot-package.h"
#include "sd-boot.h"

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
int package_version_installed(PkgInfo *pkginfo) {

    if (!pkginfo || !pkginfo->pkg_name) {
        return -1;
    }

    pkginfo->pkg_vers = get_curr_pkg_vers_alpm(pkginfo->pkg_name);
    return 0;
}
