// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Helper routine for run_cmd_output - reads child process stdout.
 *
 * Args:
 *  int fdes 
 *      - file descriptor to read from
 *
 *  char **output_p 
 *      - *output_p points to the output that was read.
 *        caller should free memory.
 *          
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sd-boot-cmd.h"
#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

enum BufSize {
    CHUNK = 4096,
};


/*
 * Make sure there's room for a full CHUNK-sized read.
 * Grow geometrically (double capacity) instead of resizing
 * to exactly what's needed on every iteration - this keeps
 * the total number of realloc()/copy operations to O(log n)
 * rather than O(n / CHUNK).
 */
static int mem_grow_if_needed(Dynamic_str *str) {
    int ret = 0;

    if (str->num_alloc - str->num_used < CHUNK) {
        size_t new_size = str->num_alloc * 2;
        size_t needed = str->num_used + CHUNK;
        if (new_size < needed) {
            new_size = needed;
        }
        ret = dynamic_str_alloc(new_size, str);
        if (ret != 0 && new_size > needed) {
            /*
             * Doubling may have overshot the allocator's size cap
             * even though the actually-needed amount is still
             * within it - fall back to requesting just that.
             */
            ret = dynamic_str_alloc(needed, str);
        }
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

int read_child_output(int fdes, char **output_p) {
    /*
     * Read child stdout and save into *output_p
     * - allocate in chunks, growing geometrically (doubling) so we
     *   only realloc() when the buffer is actually full, rather than
     *   after every single read().
     * - allocate one extra byte leaving room for null termination.
     * We realloc() mem down to what is needed once done reading.
     * If there was no output, *output_p is set to an allocated
     * empty string "".
     */
    int ret = 0;
    Dynamic_str str = {};

    /*
     * sanity
     */
    if (!output_p) {
        msg(MSG_ERR, "  ! read_child: bad output ptr\n");
        ret = -1;
        goto exit;
    }
    *output_p = nullptr;

    /*
     * First chunk of memory
     */
    ret = dynamic_str_alloc(CHUNK, &str);
    if (ret != 0) {
        goto exit;
    }

    /*
     * ptr: track where in the buffer to read the next chunk.
     */
    while (1) {
        /*
         * Make sure there's room for a full CHUNK-sized read.
         */
        ret = mem_grow_if_needed(&str);
        if (ret != 0) {
            goto exit;
        }

        char *ptr = str.bytes + str.num_used;
        ssize_t bytes_read = read(fdes, ptr, CHUNK);

        if (bytes_read < 0) {
            /*
             * Error
             */
            // NOLINTNEXTLINE(misc-include-cleaner)
            if (errno == EINTR) {
                continue;
            }
            perror("Error reading child pipe");
            ret = -1;
            goto exit;
        }

        if (bytes_read == 0) {
            /*
             * EOF
             */
            break;
        }
        str.num_used += (size_t)bytes_read;
    }

    /*
     * Resize down and add null terminator
     */
    if (str.bytes != nullptr) {
        ret = dynamic_str_alloc(str.num_used + 1, &str);
        if (ret != 0) {
            goto exit;
        }
        str.bytes[str.num_used] = '\0';
    }

    *output_p = str.bytes;
    str.bytes = nullptr;
    str.num_alloc = 0;

exit:
    if (str.bytes) {
        /*
         * error where pointer is not transferred to *output_p 
         */
        (void)dynamic_str_alloc(0, &str);
    }
    return ret;
}

