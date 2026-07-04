// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read Package Versions File
 * - /var/lib/sd-boot/<package-name>.version
 */
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-config.h"
#include "sd-boot-keyval.h"
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

/*
 * only first line with key is kept.
 */
static int raw_read_file(const char *file, char **curr_p, char **prev_p) {

    int ret = 0;
    KvList list = {};

    if (!file) {
        return 0;
    }

    /*
     * -1 => error
     *  1 => file not available
     *  0 => success
     */
    list.max_str_len = PATH_MAX;

    ret = parse_keyval_file(file, &list);
    if (ret < 0) {
        msg(MSG_ERR, "  ! sd-boot: error reading file %ss\n", file);
        return -1;
    }

    if (ret == 1) {
        return 0;
    }

    for (size_t i = 0; i < list.num_elems; i++) {
        KvElem *elem = &list.elems[i];

        if (elem->type != KV_STR) {
            continue;
        }

        if (*curr_p == nullptr && strcmp(elem->key, "current") == 0) {
            *curr_p = elem->str_val;
            elem->str_val = nullptr;
            continue;
        } 

        if (*prev_p == nullptr && strcmp(elem->key, "previous") == 0) {
            *prev_p = elem->str_val;
            elem->str_val = nullptr;
            continue;
        }
    }

    kvlist_free(&list);

    return 0;
}

/*
 * Write the package file curr/prev vers file
 */
int write_package_version_file(SdBoot *conf, PkgInfo *pkginfo) {
    int ret = 0;
    char path[PATH_MAX] = {};

    if (package_version_filename(pkginfo->pkg_name, conf->root, path, sizeof(path)) != 0) {
        return -1;
    }

    FILE *fptr = nullptr;
    fptr = fopen(path, "w");
    if (!fptr) {
        perror(nullptr);
        msg(MSG_ERR, "  ! sd-boot: error opening %s\n", path);
        return -1;
    }

    /*
     * Write the file
     */
    char *data = pkginfo->vers_curr ? pkginfo->vers_curr : "";
    if (!data) {
        data = "";
    } 
    if (fprintf(fptr, "current = %s\n", data) < 0){
        msg(MSG_ERR, "  ! sd-boot: error writing %s\n", path);
        ret = -1;
        goto exit;
    }

    data = pkginfo->vers_prev ? pkginfo->vers_prev : "";
    if (fprintf(fptr, "previous = %s\n", data) < 0){
        msg(MSG_ERR, "  ! sd-boot: error writing %s", path);
        ret = -1;
        goto exit;
    }

exit:
    if (fptr) {
        if (fclose(fptr) != 0) {
            perror(nullptr);
        }
    } 
    return ret;
}

/*
 * File has 2 rows:
 * current = xxx
 * previoius = xxx
 *
 * Returns (same as read_kv_elems:
 *   -1 = error
 *    0 = success reading num_elems
 *    1 = no file to read
 */
int read_package_version_file(SdBoot *conf, PkgInfo *pkginfo) {
    int ret = 0;

    if (!pkginfo || !pkginfo->pkg_name) {
        return -1;
    }

    /*
     * Fresh start
     */
    if (pkginfo->vers_curr) {
        free((void *)pkginfo->vers_curr);
        pkginfo->vers_curr = nullptr;
    }

    if (pkginfo->vers_prev) {
        free((void *)pkginfo->vers_prev);
        pkginfo->vers_prev = nullptr;
    }

    /*
     * Get data from file
     */
    char path[PATH_MAX] = {};
    if (package_version_filename(pkginfo->pkg_name, conf->root, path, sizeof(path)) != 0) {
        ret = -1;
        goto exit;
    }

    ret = raw_read_file((const char *)path, &pkginfo->vers_curr, &pkginfo->vers_prev);
    if (ret != 0) {
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
