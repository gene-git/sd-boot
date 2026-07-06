// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Initilize spawn attributes and reset child signals.
 *
 * Note: caller must clean up with posix_spawnattr_destroy() 
 *       with or withour errors.
 */
#include <signal.h>
#include <spawn.h>

#include "sd-boot-cmd.h"


int init_spawn_attr(posix_spawnattr_t *attr) {
    int ret = 0;
    sigset_t sig_mask = {};                 // NOLINT(misc-include-cleaner)

    ret = posix_spawnattr_init(attr);
    if (ret != 0) {
        goto exit;
    }

    ret = sigemptyset(&sig_mask);
    if (ret != 0) {
        goto exit;
    }

    ret = posix_spawnattr_setsigmask(attr, &sig_mask);
    if (ret != 0) {
        goto exit;
    }

    ret = posix_spawnattr_setflags(attr, POSIX_SPAWN_SETSIGMASK);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}
