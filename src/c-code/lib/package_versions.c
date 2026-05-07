// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read Package Versions File
 * - /var/lib/sd-boot/<pkgbase>.version
 */
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

static int package_vers_filename(const char *pkg, char *root, char *path, size_t len_path) {
    /*
     * Filename which holds curr/prev version info
     */
    int ret = 0;
    path[0] = '\0';

    /*
     * Make sure dirs exist
     */
    char path_dir[PATH_MAX] = {'\0'};

    if (snprintf(path_dir, len_path, "%s/%s", root, "var/lib/sd-boot") < 0) {
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

static int write_package_versions(char *root, PackageVersion *pkg_vers) {
    /*
     * Write the package file curr/prev vers file
     */
    int ret = 0;
    char path[PATH_MAX] = {'\0'};

    if (package_vers_filename(pkg_vers->pkg, root, path, sizeof(path)) != 0) {
        return -1;
    }

    FILE *fptr = nullptr;
    fptr = fopen(path, "w");
    if (fptr == nullptr) {
        perror(nullptr);
        msg(MSG_ERR, "  sd-boot: error opening %s\n", path);
        return -1;
    }
    /*
     * Write the file
     */
    if (fprintf(fptr, "current = %s\n", pkg_vers->current) < 0){
        msg(MSG_ERR, "  sd-boot: error writing %s\n", path);
        ret = -1;
        goto exit;
    }

    if (fprintf(fptr, "previous = %s\n", pkg_vers->previous) < 0){
        msg(MSG_ERR, "  sd-boot: error writing %s", path);
        ret = -1;
        goto exit;
    }

exit:
    if (fptr != nullptr) {
        if (fclose(fptr) != 0) {
            perror(nullptr);
        }
    } 
    return ret;
}

int read_package_versions(SdBoot *conf, const char *pkg, PackageVersion *pkg_vers) {
    /*
     * Read the package version file which populates 
     * - pkg_vers->current
     * - pkg_vers->previous
     * Returns:
     *  -1 = error
     *   0 = all good
     *   1 = no file no version
    */ 
    int ret = 0;
    int sret = 0;
    size_t num_elems = 2;
    KvElem *elem = nullptr;

    if (pkg == nullptr || *pkg == '\0' || pkg_vers == nullptr) {
        ret = -1;
        goto exit;
    }
    
    strncpy(pkg_vers->pkg, pkg, MAX_VAL_LEN-1);

    sret = alloc_kv_elems(num_elems, &elem);
    if (sret != 0) {
        msg(MSG_ERR, "sd-boot: mem alloc fail in read_package_versions\n");
        ret = -1;
        goto exit;
    }

    elem[0].key = "current";
    elem[0].type = CONF_STR;
    elem[0].val.v_str[0] = '\0';

    elem[1].key = "previous";
    elem[1].type = CONF_STR;
    elem[0].val.v_str[0] = '\0';

    char path[PATH_MAX] = {'\0'};
    if (package_vers_filename(pkg, conf->info.root, path, sizeof(path)) != 0) {
        ret = -1;
        goto exit;
    }

    size_t num_elems_read = 0;
    sret = read_kv_elems(path, num_elems, elem, &num_elems_read);
    switch (sret) {
        case -1:
            msg(MSG_ERR, "sd-boot: mem alloc fail in read_package_versions\n");
            ret = -1;
            goto exit;
            break;

        case 0:
            // all good
            if (num_elems_read > 0) {
                strncpy(pkg_vers->current, elem[0].val.v_str, MAX_VAL_LEN);
                if (num_elems_read > 1) {
                    strncpy(pkg_vers->previous, elem[1].val.v_str, MAX_VAL_LEN);
                }
            }
            ret = 0;
            break;

        case 1:
        default:
            // no package file available
            ret = 1;
            break;
    }

exit:
    if (elem != nullptr) {
        free((void *)elem);
    }
    return ret;
}

int update_package_versions(SdBoot *conf, const char *pkg, PackageVersion *pkg_vers) {
    /*
     * Update package versions and save to the file
     * - pkg_vers->current
     * - pkg_vers->previous
     * 
     * - if pkg_vers->current[0] == '\0' then pacman is used to lookup
     *   the package version installed. If "current" has a version then
     *   it is used.
     *   kernel packacges pass in the version (since it is known) while
     *   efi-tools do not.
     */
    int ret = 0;
    strncpy(pkg_vers->pkg, pkg, MAX_VAL_LEN-1);

    /*
     * read file so to get it's view of current and if it is
     * different than the current version we update.
     */
    PackageVersion pkg_vers_now = {};
    int sret = read_package_versions(conf, pkg, &pkg_vers_now);
    bool got_now_vers = false;
    switch (sret) {
        case -1:
            msg(MSG_ERR, "  sd-boot: error reading pkg vers %s\n", pkg);
            ret = -1;
            goto exit;
            break;

        case 0:
            got_now_vers = true;
            break;

        case 1:
        default:
            // nothing found
            break;
    }

    /*
     * If not passed in then get current installed package version.
     * kernel gets the "version" from /usr/lib/modules not the package version
     * and these are not the same (e.g. no hyphens in package vers)
     * For efi tools we choose to use the package version by 
     */
    if (pkg_vers->current[0] == '\0') {
        sret = package_version_installed(pkg, MAX_VAL_LEN, pkg_vers->current);
        if (sret != 0) {
            ret = -1;
            goto exit;
        }
    }

    /*
     * Check if current version is different than in the file (if got file info)
     */
    bool updated = false;
    if (got_now_vers) {
        // "now current" becomes previous
        if (strncmp(pkg_vers->current, pkg_vers_now.current, MAX_VAL_LEN+1) != 0) {
            strncpy(pkg_vers->previous, pkg_vers_now.current, MAX_VAL_LEN+1);
            updated = true;
        }
    } else {
        updated = true;
    }

    if (updated) {
        sret = write_package_versions(conf->info.root, pkg_vers);
        if (sret < 0) {
            ret = -1;
            goto exit;
        }
    }

exit:
    return ret;
}

int remove_package_versions(SdBoot *conf, const char *pkg) {
    /*
     * Remove the package version file
     */
    int ret = 0;
    char path[PATH_MAX] = {'\0'};

    if (package_vers_filename(pkg, conf->info.root, path, sizeof(path)) != 0) {
        return -1;
    }

    if (remove_file(path) != 0) {
        msg(MSG_ERR, "  sd-boot: error removing file: %s\n", path);
    }
    return ret;
}


