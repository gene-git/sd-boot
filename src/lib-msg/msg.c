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

void set_verb_level(int verb_level) {
    /*
     * Set message level
     */
    if (verb_level < MSG_ERR) {
        VerbLevel = MSG_ERR;
    } else if (verb_level > MSG_QUIET) {
        VerbLevel = MSG_QUIET;
    } else {
        VerbLevel = verb_level;
    }
}

/**
 * For level MSG_ERR then print to stderr otherwise to stdout
 */
[[gnu::format(printf, 2, 3)]]
void msg(int level, const char *fmt, ...) {

    if (!fmt ) {
        return;
    }
    if (level <= VerbLevel) {
        int num = 0;
        FILE *stream = stdout;
        if (level == MSG_ERR) {
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

