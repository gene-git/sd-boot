// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Determine if a trigger (string provided via alpm hook) is 
 * of type = Path.
 *
 * Simple check: if "/" is in trigger name => its a path.
 * Will not catch triggers of the form: etc
 * But then again, this should never be used.
 */
#include <stdbool.h>
#include <string.h>

bool trigger_is_path(const char *trigger) {
    if (!trigger) {
        return false;
    }

    if (strchr(trigger, '/')) {
        return true;
    }
    return false;
}

