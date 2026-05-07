// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read one line from file descriptor
 */
#include <stddef.h>
#include <unistd.h>

#include "sd-boot.h"

int read_one_line_fd(int fdes, char *buf, size_t len) {
    /*
     * Read from file descriptor one character at a time.
     * Return
     * 0 if reached end of line
     * 1 if reached end of file
     * -1 (buf too small)
     */
    char one_char = '\0';
    int ret = 0;
    size_t idx = 0;

    size_t count = 1;
    while (count == 1) {
        /*
         * read returns
         *  1 = 1 character read (N in the general case)
         *  0 = end of file
         * -1 = error
         */
        count = read(fdes, &one_char, 1);

        if (count == 0) {
            /*
             * end of file
             */
            ret = 1;
            break;
        }

        if (count < 1) {
            /* 
             * error
             */
            ret = -1;
            break;

        } if (count > 0) {
            /* 
             * read 1 character
             */
            ret = 0;
        }

        if (one_char == '\n') {
             /*
              * end of line
              */
            buf[idx] = '\0';
            break;
        }

        if (idx < len - 1) {
            buf[idx++] = one_char;
        } else {
            /* 
             * buf too small
             */
            ret = -1;
            break;
        }
    }
    return ret;
}
