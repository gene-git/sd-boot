// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read one line from file descriptor
 */
#include <stddef.h>
#include <unistd.h>


/*
 * Read from file descriptor one character at a time.
 * Buf is null terminated 
 * Return
 * 0 if reached end of line
 * 1 if reached end of file
 * -1 (buf too small)
 */
int read_one_line_fd(int fdes, char *buf, size_t len) {
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

        /*
         * end of file 
         * - keep any characters in buf.
         */
        if (count == 0) {
            if (idx > 0) {
                buf[idx] = '\0';
                ret = 0;
            } else {
                ret = 1;
            }
            break;
        }

        if (count < 1) {
            ret = -1;
            break;

        } 
        
        /*
         * read one char
         */
        if (count > 0) {
            ret = 0;
        }

        /*
         * end of line
         */
        if (one_char == '\n') {
            buf[idx] = '\0';
            break;
        }

        /*
         * buf size check
         */
        if (idx < len - 1) {
            buf[idx++] = one_char;
        } else {
            ret = -1;
            break;
        }
    }
    return ret;
}
