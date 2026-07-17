// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read Package Versions File
 * - /var/lib/sd-boot/<package-name>.version
 */
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-msg.h"
#include "sd-boot-package.h"
#include "sd-boot-utils.h"

/*
 * Filename with curr/prev versions
 */
static int package_version_filename(const char *pkg, char *root, char *path, size_t len_path) {
    int ret = 0;
    path[0] = '\0';

    /*
     * Make sure dirs exist
     */
    char path_dir[PATH_MAX] = {};

    if (snprintf(path_dir, len_path, "%s%s", root, "var/lib/sd-boot") < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    ret = makepath(path_dir, MKDIR_MODE_DEF);
    if (ret != 0) {
        goto exit;
    }

    /*
     * path ~ /var/lib/sd-boot/<pkg>.version
     */
    if (snprintf(path, len_path, "%s/%s.version", path_dir, pkg) < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
exit:
    return ret;
}

int remove_package_version_file(SdBoot *conf, PkgInfo *pkginfo) {
    int ret = 0;
    char path[PATH_MAX] = {};

    if (!pkginfo || !pkginfo->pkg_name) {
        return -1;
    }

    if (package_version_filename(pkginfo->pkg_name, conf->root, path, sizeof(path)) != 0) {
        ret = -1;
        goto exit;
    }

    if (remove_file((const char *)path) != 0) {
        msg(MSG_ERR, "  ! sd-boot: error removing file: %s\n", path);
    }

exit:
    return ret;
}
