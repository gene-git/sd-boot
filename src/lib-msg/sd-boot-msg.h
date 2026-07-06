// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-msg.h
 */
#ifndef SD_BOOT_MSG_H
#define SD_BOOT_MSG_H

#include "sd-boot-export.h"

/*
 *
 * Message Options (Verbosity)
 */
enum MsgVerb {
    MSG_ERR = -1,
    MSG_VERB = 0,
    MSG_NORMAL = 1,
    MSG_QUIET = 2,
};


/*
 * Function declarations
 */
void set_verb_level(int verb_level);

/*
 * Public API
 */
[[gnu::format(printf, 2, 3)]]
SD_BOOT_EXPORT void msg(int msg_level, const char *fmt, ...);

#endif
