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
    ret = makepath(path_dir, 0);
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
 * Allocate and copy elem value
 */
static char *elem_to_string_value(KvElem *elem) {
    char *value = nullptr;

    if (elem->val.v_str[0] != '\0') {
        value = strdup(elem->val.v_str);
    }
    return value;
}

static int raw_read_file(const char *file, char **curr_p, char **prev_p) {

    int ret = 0;
    size_t num_elems = 2;
    KvElem *elem = nullptr;

    /*
     * Want 2 elems from file
     */
    ret = alloc_kv_elems(num_elems, &elem);
    if (ret != 0) {
        msg(MSG_ERR, "  ! sd-boot: mem alloc fail in read_package_versions\n");
        ret = -1;
        goto exit;
    }

    elem[0].key = "current";
    elem[0].type = CONF_STR;
    elem[0].val.v_str[0] = '\0';

    elem[1].key = "previous";
    elem[1].type = CONF_STR;
    elem[1].val.v_str[0] = '\0';

    size_t num_read = 0;
    ret = read_kv_elems(file, num_elems, elem, &num_read);
    switch (ret) {
        case -1:
            msg(MSG_ERR, "  ! sd-boot: read_package_versions error reading file\n");
            ret = -1;
            goto exit;
            break;

        case 0:
            /*
             *  all good
             */
            if (num_read > 0) {
                *curr_p = elem_to_string_value(&elem[0]);

                if (num_read > 1) {
                    *prev_p = elem_to_string_value(&elem[1]);
                }
            }
            ret = 0;
            break;

        case 1:
        default:
            /* 
             * no package file available
             */
            ret = 0;
            break;
    }

exit:
    if (elem) {
        free((void *)elem);
    }
    return ret;
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

    if (remove_file(path) != 0) {
        msg(MSG_ERR, "  ! sd-boot: error removing file: %s\n", path);
    }

exit:
    return ret;
}
