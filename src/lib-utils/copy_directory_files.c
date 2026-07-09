// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Copy files under src_dir to dst_dir.
 * Non recursive - only the files directly under the src_dir.
 * Only copies files (not links).
 */
#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"


/*
 * We Ignore non-files.
 * d_type isn't guaranteed to be populated by every filesystem
 * (some report DT_UNKNOWN) - fall back to stat() on the source
 * path in that case. Only the source entry's type matters here;
 * dst_path doesn't need checking.
 */
static int check_is_regular_file(struct dirent *entry, char *src_path, bool *is_reg_file) {

    *is_reg_file = false;
    if (entry->d_type == DT_REG) {
        *is_reg_file = true;

    } else if (entry->d_type == DT_UNKNOWN) {
        struct stat stat_buf = {};
        if (stat(src_path, &stat_buf) < 0) {
            perror("stat failed");
            return -1;
        }
        *is_reg_file = S_ISREG(stat_buf.st_mode);
    }

    return 0;
}

static int make_pathname(const char *dir, char *name, char *path) {

    int len = snprintf(path, PATH_MAX, "%s/%s", dir, name);
    if (len < 0 || len >= PATH_MAX) {
        msg(MSG_ERR, "!  sd-boot error creating path name to copy\n");
        return -1;
    }
    return 0;
}

static int alloc_paths(char **src_path_p, char **dst_path_p) {

    *src_path_p = calloc(PATH_MAX, sizeof(char));
    if (*src_path_p == nullptr) {
        msg(MSG_ERR, "!  sd-boot failed to allocate mem\n");
        return -1;
    }

    *dst_path_p = calloc(PATH_MAX, sizeof(char));
    if (*dst_path_p == nullptr) {
        msg(MSG_ERR, "!  sd-boot failed to allocate mem\n");
        return -1;
    }
    return 0;
}

static int do_copy(char *src_path, char *dst_path, bool use_fast) {

    if (use_fast) {
        return copy_file_fast(src_path, dst_path, WRITE_FLAGS_DEF, WRITE_MODE_DEF);
    }
    return copy_file(src_path, dst_path);
}

/*
 * Args:
 *  src:    directory to copy from
 *  dst:    copy src/<file> to dst/<file>
 *  use_fast: if trye, use copy_file_fast() which uses copy_file_range().
 *          This has limitations (e.g. cross filesystem/cross device etc). 
 *          See man copy_file_range()
 *          If unsure use false.
 */
int copy_directory_files(const char *src, const char *dst, bool use_fast) {

    int ret = 0;
    if (!src || !dst) {
        return -1;
    }

    /*
     * Make sure dest dir exists
     */
    if (makedir(dst, MKDIR_MODE_DEF) < 0) {
        return -1;
    }

    /*
     * Prep 
     */
    DIR *dir_src = opendir(src);
    if (!dir_src) {
        perror("opendir failed");
        return -1;
    }

    char *dst_path = nullptr;
    char *src_path = nullptr;
    ret   = alloc_paths(&src_path, &dst_path);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Get files and copy them
     */
    struct dirent *entry = nullptr;
    
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    while ((entry = readdir(dir_src))) {

        /*
         * Build src_path first - we may need to stat() it below to
         * determine the entry's type when d_type isn't populated.
         */
        ret = make_pathname(src, entry->d_name, src_path);
        if (ret != 0) {
            goto exit;
        }

        /*
         * Ignore non-files.
         */
        bool is_reg_file = false;
        if (check_is_regular_file(entry, src_path, &is_reg_file) < 0) {
            ret = -1;
            goto exit;
        }

        if (!is_reg_file) {
            continue;
        }

        ret = make_pathname(dst, entry->d_name, dst_path);
        if (ret != 0) {
            goto exit;
        }

        ret = do_copy(src_path, dst_path, use_fast);
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    if (src_path) {
        free((void *)src_path);
    }

    if (dst_path) {
        free((void *)dst_path);
    }

    if (dir_src) {
        (void)closedir(dir_src);
    }

    return ret;
}
