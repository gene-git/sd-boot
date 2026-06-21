// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Permission check.
 * For production root is required.
 * In development testing mode it is not.
 */
#include <stdbool.h>

#include "sd-boot-msg.h"
#include "sd-boot-config.h"

bool check_permission(SdBoot *conf) {
    if (!conf->test && conf->euid != 0) {
        msg(MSG_ERR, "! root permission requiredd.\n");
        return false;
    }
    return true;
}
