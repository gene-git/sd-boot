// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Display a message string to terminal.
 * Supports verbosity level.
 */ 
#include <stdarg.h>
#include <stdio.h>      

#include "sd-boot-msg.h"

/*
 * Non-mt is fine for this pupose
 */
static int VerbLevel = MSG_NORMAL; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

/**
 * Set message level
 * - permitted values are from 0 to 3 
 *                                          Message Level
 *                          MSG_ERR     MSG_NORM    MSG_VERB    "kernel-install -v"
 *   verb-level             0           1           2           3 
 *   0 = quiet              Y
 *   1 = normal             Y           Y
 *   2 = verbose            Y           Y           Y
 *   3 = very verbose       Y           Y           Y           Y
 *
 */
void set_verb_level(int verb_level) {

    if (verb_level < MSG_ERR) {
        VerbLevel = MSG_ERR;

    } else if (verb_level > MSG_VERB + 1) {
        VerbLevel = MSG_VERB + 1;

    } else {
        VerbLevel = verb_level;
    }
}

/**
 * For MSG_ERR goes to stderr otherwise to stdout
 */
[[gnu::format(printf, 2, 3)]]
void msg(int msg_level, const char *fmt, ...) {

    if (!fmt ) {
        return;
    }
    if (VerbLevel >= msg_level) {
        int num = 0;
        FILE *stream = stdout;
        if (msg_level == MSG_ERR) {
            stream = stderr;
        }

        // NOLINTBEGIN(clang-analyzer-security.VAList)
        va_list args;
        va_start(args, fmt);
        num = vfprintf(stream, fmt, args);
        va_end(args);
        // NOLINTEND(clang-analyzer-security.VAList)
        
        (void)fflush(stream);

        if (num < 0) {
            perror(nullptr);
            (void)fflush(stream);
        }
    }
}

