// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Copy a file to another location
 */
#include <stdio.h>
#include <stdlib.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

enum BufferSize {
    CHUNK = 4096,
};

int copy_file(const char *src, const char *dst) {

    int ret = 0;
    FILE *fptr_src = nullptr;
    FILE *fptr_dst = nullptr;

    fptr_src = fopen(src, "rb");
    if (!fptr_src) {;
        perror(nullptr);
        return -1;
    }

    fptr_dst = fopen(dst, "wb");
    if (!fptr_dst) {;
        perror(nullptr);
        (void)fclose(fptr_src);
        return -1;
    }

    unsigned char buf[CHUNK] = {};
    const size_t buf_size = sizeof(buf[0]);
    const size_t buf_count = sizeof(buf) / buf_size;
    const size_t buf_bytes = buf_size * buf_count;
    size_t bytes_in = 0;
    size_t bytes_out = 0;

    while ((bytes_in = fread(buf, buf_size, buf_count, fptr_src)) > 0) {

        bytes_out = fwrite(buf, buf_size, bytes_in, fptr_dst);

        /*
         * read: check for end of file or error 
         */
        if (bytes_in < buf_bytes) {
            if (ferror(fptr_src)) {
                perror("fread fail");
                ret = -1;
                break;
            }

            if (feof(fptr_src)) {
                break;
            }
        }

        /*
         * write: check for error
         */
        if (bytes_out < bytes_in) {
            msg(MSG_ERR, "  fwrite error - disk space?\n");
            ret = -1;
            break;
        }
    }

    if (fclose(fptr_src) != 0) {
        perror("fclose error");
        ret = -1;
    };

    if (fclose(fptr_dst) != 0) {
        perror("fclose error");
        ret = -1;
    };

    return ret;
}

