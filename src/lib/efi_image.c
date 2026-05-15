// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * read the file etc/sd-boot/<pkg>.image
 * which contains the file path.
 *
 * Contents are comments (# ...)
 * first non-comment line is the path
 */
#include <glob.h>
#include <libgen.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

char *package_to_efi_image(SdBoot *conf, char *pkg) {
    /*
     * Given package name of an efi tool, find the path
     * to the source efi image.
     *
     * Returns nullptr if not found.
     *
     * Reads the file /etc/sd-boot/<package-name>.image (if it exists)
     *
     * efi_path is allocated - caller must free()
     * 
     */
    char path[PATH_MAX] = {'\0'};
    char *efi_path = nullptr;

    if (snprintf(path, sizeof(path), "%s/%s%s.image", conf->info.root, "etc/sd-boot/", pkg) < 0) {
        perror(nullptr);
    } else {
        efi_path = read_file_first_row((const char *)path);
    }
    if (efi_path == nullptr) {
        msg(MSG_ERR, "  sd-boot: Failed to load efi package file  %s\n", path);
    }

    return efi_path;
}

char *efi_image_to_package(SdBoot *conf, const char *path) {
    /*
     * Find package name and return it
     *
     * Args:
     *  path = path to image file.
     *  pkg  = array of size "size" where package name is copied to
     *  size = size of pkg array.
     *
     * Returns:
     *  package name (mem is malloc)or nullptr if not found.
     *  Caller should free the mem.
     */
    char *pkg = nullptr;
    if (path == nullptr || *path == '\0') {
        goto exit;
    }

    /*
     * Read all files in /etc/sd-boot/ 
     *   named: *.image
     * - check if image path matches.
     * - should never be duplicates - for safety we take first matching package
     */
    glob_t gstruct = {};
    char pattern[PATH_MAX] = {'\0'};
    char *this_path = nullptr;
    char *filename = nullptr;
    char *file = nullptr;

    if (snprintf(pattern, sizeof(pattern), "%s/%s.image", conf->info.root, "etc/sd-boot/*") < 0) {
        perror(nullptr);
        goto exit;
    }
    int ret = glob(pattern, 0, nullptr, &gstruct);  // NOLINT(concurrency-mt-unsafe)

    if (ret == 0) {
        bool matched = false;

        for (size_t idx = 0; ! matched && idx < gstruct.gl_pathc; idx++) {

            this_path = read_file_first_row((const char *)gstruct.gl_pathv[idx]);

            if (this_path != nullptr && strcmp(this_path, path) == 0) {
                filename = gstruct.gl_pathv[idx];
                file = basename(filename);
                strip_file_extension(file, ".image");
                pkg = strdup(file);
                matched = true;
            }
            free((void *)this_path);
            this_path = nullptr;
        }
        globfree(&gstruct);
    }

exit:
    return pkg;
}
