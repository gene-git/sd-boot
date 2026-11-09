// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Display character buffer on stdout:
 *
 * Allow output to be shifted right to faciliate formatting.
 * Ouptut is shown at verb level msg_level or higher.
 * Words a not split so output can be longer than line_len.
 *
 * :param msg_level: The verbosity level to show the buffer. e.g. MSG_NORM, MSG_VERB
 * :param margin: Number of spaces to prepend each line
 * :param line_len: Max number of characters to show per line beyond the margin. No limit if < 0
 * :param buffer: The text buffer to show (may have newlines).
 */
#include "sd-boot-msg.h"

#include <stdint.h>
#include <stdio.h>

void msg_buf(int msg_level, uint16_t margin, int line_len, const char *buffer) {
    int verb_level = get_verb_level();

    if (!buffer || verb_level < msg_level) {
        return;
    }

    const char *ptr = buffer;

    while (*ptr != '\0') {
        for (uint16_t i = 0; i < margin; i++) {
            putchar(' ');
        }

        int count = 0;
        while (*ptr != '\0' && *ptr != '\n') {
            if (line_len > 0 && count >= line_len) {
                break;
            }
            putchar(*ptr++);
            count++;
        }

        while (*ptr != ' ' && *ptr != '\0' && *ptr != '\n') {
            putchar(*ptr++);
            count++;
        }

        if (*ptr == '\n') {
            ptr++;
        }
        putchar('\n');
        (void)fflush(stdout);
    }
}
