// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Display a message string to stderr.
 * Suport simple verbosity level.
 */ 
#include <stdarg.h>
#include <stdio.h>      

#include "sd-boot.h"

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

void msg(int msg_level, const char *fmt, ...) {
    if (fmt == nullptr) {
        return;
    }
    if (msg_level <= VerbLevel) {
        int num = 0;
        // NOLINTBEGIN(clang-analyzer-security.VAList)
        va_list args;
        va_start(args, fmt);
        num = vprintf(fmt, args);
        va_end(args);
        // NOLINTEND(clang-analyzer-security.VAList)
        
        (void)fflush(stdout);

        if (num < 0) {
            perror(nullptr);
            (void)fflush(stderr);
        }
    }
}
