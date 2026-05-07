// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Remove directory and it's contents
 */
#define XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <sys/stat.h>

enum Sizes {
    OPEN_FDS = 64,
};

/*
 * ntfw Callback
 * remove() is convenient as it deletes files and empty direcrories.
 * Saves doing stat then unlink or rmdir
 */
static int unlink_cb(const char *fpath, const struct stat *stat_buf, int typeflag, struct FTW *ftwbuf) {
    // tell compiler these are unused
    (void)stat_buf;
    (void)typeflag;
    (void)ftwbuf;

    int ret = remove(fpath);
    if (ret) {
        perror(fpath);
    }
    return ret;
}

int rm_rf(const char *path) {
    /*
     * Returns 0 on success.
     * FTW_DEPTH: Post-order traversal (children before parents).
     * FTW_PHYS: Do not follow symbolic links (delete the link itself).
     * 64: Maximum number of open file descriptors.
     */
    return nftw(path, unlink_cb, OPEN_FDS, FTW_DEPTH | FTW_PHYS);   // NOLINT(concurrency-mt-unsafe)
}

