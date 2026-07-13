// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/*
 * Copy src_file to dst_file using copy_file_range() 
 * Supported on all kernels 5.19+ but not for all device / filesystems.
 * If not supported we fall back to regular userspace read / write copy.
 *
 * Not all device/filesystem configurations are supported by the kernel.
 * For example if src and dst are not on  the  same  filesystem and the 
 * filesystems are not of the same type or filesystem(s) do not support cross-filesystem copy.
 */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"


static int close_fds(int *fd_src, int *fd_dst) {

    int ret = 0;

    if (fd_src && *fd_src >= 0) {
        if (close(*fd_src) != 0) {
            perror("copy_file_fast: Error closing src file");
            ret = -1;
        }
        *fd_src = -1;
    }

    if (fd_dst && *fd_dst >= 0) {
        if (close(*fd_dst) != 0) {
            perror("copy_file_fast: Error closing dst file");
            ret = -1;
        }
        *fd_dst = -1;
    }

    return ret;
}

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
    bool use_fallback = false;

    // NOLINTBEGIN(misc-include-cleaner)
    while (true) {
        bytes_copied = copy_file_range(fd_src, nullptr, fd_dst, nullptr, SSIZE_MAX, copy_flags);

        if (bytes_copied < 0) {
            if (errno == EINTR) {
                continue;
            }

            if (errno == EXDEV || errno == ENOSYS || errno == EOPNOTSUPP) {
                use_fallback = true;
                break;
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
    // NOLINTEND(misc-include-cleaner)
    
    /* 
     * copy_file_range not available - use standard read/write
     */
    if (use_fallback) {

        msg(MSG_VERB, " copy_file_fast unavailable - using read/write\n");

        if (close_fds(&fd_src, &fd_dst) != 0) {
            ret = -1;
        }

        if (ret == 0) {
            ret = copy_file(src, dst);
        }
    }

exit:
    if (close_fds(&fd_src, &fd_dst) != 0) {
        ret = -1;
    }
    return ret;
}
