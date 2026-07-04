// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Given a source directory, make symlinks dst/link -> src/link
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sd-boot-utils.h"

static int do_one_link(const char *src, int dst_fd, const char *name) {

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return 0;
    }

    /*
     * Build target path for src/name
     */
    char target_path[PATH_MAX] = {};
    int len = snprintf(target_path, sizeof(target_path), "%s/%s", src, name);

    if (len < 0 || (size_t)len >= sizeof(target_path)) {
        return -1;
    }

    
    /*
     * Create the symlink: 'dst/name' -> src/name (target_path)
     */
    if (symlinkat(target_path, dst_fd, name) == -1) {
        // NOLINTNEXTLINE(misc-include-cleaner)
        if (errno == EEXIST) {

            /*
             * Read link target
             * - it is not null terminated.
             */
            char existing_target[PATH_MAX] = {};
            ssize_t nbytes = readlinkat(dst_fd, name, existing_target, sizeof(existing_target) - 1);

            /*
             * sanitize nbytes (dont trust it - we have no control what the link target is)
             */
            if (nbytes < 0 || (size_t)nbytes >= sizeof(existing_target)) {
                return -1;
            }

            existing_target[nbytes] = '\0';

            /*
             * target is same as existing
             */
            if (strcmp(existing_target, target_path) == 0) {
                return 0;
            }

            /*
             * Wrong link
             */
            if (unlinkat(dst_fd, name, 0) == -1) {
                return -1;
            }

            /*
             *  Fix link
             */
            if (symlinkat(target_path, dst_fd, name) == -1) {
                return -1;
            }
        } else {
            // failed for some  other reason EEXIST
            return -1;
        }
    }
    return 0;
}

/*
 * Make a shadow directory with symlinks to all files dst/x -> src/x
 * - Both src and dst must be full absolute path not relative
 */
int dir_dup_links(const char *src, const char *dst, Array_str *skips) {


    if (!src || !dst) {
        return -1;
    }

    /*
     * Make sure dst dir exists
     */
    if (makedir(dst, MKDIR_MODE_DEF) != 0) {
        return -1;
    }

    /*
     *  Open src/dir
     */
    DIR *src_dir = opendir(src);
    if (!src_dir) {
        perror("opendir failed");
        return -1;
    }

    int dst_fd = open(dst, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dst_fd == -1) {
        perror("open failed");
        (void)closedir(src_dir);
        return -1;
    }

    int ret = 0;
    struct dirent *entry = nullptr;

    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    while ((entry = readdir(src_dir))) {

        const char *name = entry->d_name;

        if (skips && skips->rows && skips->num_rows > 0) {
            if (string_in_list(name, skips->num_rows, skips->rows)) {
                continue;
            }
        }

        ret = do_one_link(src, dst_fd, name);
        if (ret != 0) {
            break;
        }
    }

    if (dst_fd != -1) {
        if (close(dst_fd) != 0) {
            perror("close failed");
            ret = -1;
        }
    }

    if (closedir(src_dir) != 0) {
        perror("closedir failed");
        ret = -1;
    }

    return ret;
}

