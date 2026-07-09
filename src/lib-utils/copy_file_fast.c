// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Copy src_file to dst_file using copy_file_range() 
 * Supported on all kernels 5.19+
 */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "sd-boot-utils.h"

/**
 * Copy file src to dst.
 *
 * Args:
 *  src:   File pathname to copy
 *  dst:   Where to write the copy
 *  flags: flags passed to open() on the destination file. 
 *  mode:  permisions on dst_file
 */
int copy_file_fast(const char *src, const char *dst, int flags, mode_t mode) {

    int ret = 0;

    if (!src || !dst) {
        return -1;
    }

    int fd_src = open(src, O_RDONLY | O_CLOEXEC);
    if (fd_src < 0) {
        perror("Error opening input file");
        return -1;
    }

    int fd_dst = open(dst, flags, mode);
    if (fd_dst < 0) {
        perror("Error creating output file");
        (void)close(fd_src);
        return -1;
    }

    /* 
     * kernel copy is efficent.
     */
    unsigned int copy_flags = 0;
    ssize_t bytes_copied = 0;

    while (true) {
        // NOLINTNEXTLINE(misc-include-cleaner)
        bytes_copied = copy_file_range(fd_src, nullptr, fd_dst, nullptr, SSIZE_MAX, copy_flags);

        if (bytes_copied < 0) {
            if (errno == EINTR) {       // NOLINT(misc-include-cleaner)
                continue;
            }
            perror("copy_file_range failed");
            ret = -1;
            goto exit;
        }

        if (bytes_copied == 0) {
            /*
             * kernel reports 0 bytes remaining - done
             */
            break;
        }
    }

exit:
    if (fd_src >= 0) {
        (void)close(fd_src);
    }
    if (fd_dst >= 0) {
        (void)close(fd_dst);
    }
    return ret;
}
