// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Check if "efivars" are available. 
 *
 * Often not the case when we're being run inside a chroot
 * For example building a package in a chroot the tests are run.
 *
 * In chroot the directory /sys/firmware/efi/efivars may exist but be empty.
 *
 * Return true if efivars are available
 */
#include <dirent.h>
#include <stdbool.h>
#include <string.h>

bool efivars_available() {

    DIR *dir = nullptr;
    struct dirent *entry = nullptr;
    const char *efi_var_dir = "/sys/firmware/efi/efivars";
    bool has_efivars = false;

    /*
     * Check dir
     */
    dir = opendir(efi_var_dir);
    if (!dir) {
        return false;
    }

    /*
     * Check content
     */
    // NOLINTNEXTLINE(concurrency-mt-unsafe
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        has_efivars = true;
        break;
    }

    if (dir) {
        (void)closedir(dir);
    }
    return has_efivars;
} 
