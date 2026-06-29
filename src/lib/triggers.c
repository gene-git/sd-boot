// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Free up the mem for TriggerInfo
 */

#include "sd-boot.h"
#include "sd-boot-utils.h"

void triggerinfo_free(TriggerInfo *trig) {

    array_str_free(&trig->pkgs);
    array_str_free(&trig->paths);

    array_str_free(&trig->pkgs_special);
    array_str_free(&trig->pkgs_other);

    array_str_free(&trig->paths_special);
    array_str_free(&trig->paths_other);
}
