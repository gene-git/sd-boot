// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Allocate and Free Memory of UpdateWork
 */
#include <stdlib.h>
#include <string.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"


static void tool_free_pkginfo(Tool *tool) {
    if (!tool->pkginfo) {
        return;
    } 
    for (size_t i = 0; i < tool->num_pkgs; i++) {
        pkginfo_free(&tool->pkginfo[i]);
    }

    free((void *)tool->pkginfo);
    tool->pkginfo = nullptr;
    tool->num_pkgs = 0;

}

int tool_alloc(size_t num_pkgs, Tool *tool) {
    int ret = 0;
    void *ptr = nullptr;

    ptr = calloc(num_pkgs, sizeof(PkgInfo));
    if (!ptr) {
        ret = -1;
        goto exit;
    }

    if (tool->pkginfo) {
        tool_free_pkginfo(tool);
    }

    tool->pkginfo = ptr;
    tool->num_pkgs = num_pkgs;

exit:
    return ret;
}

void tool_free(Tool *tool) {

    if (!tool) {
        return;
    }

    config_clean(&tool->conf);

    if (tool->pkginfo) {
        tool_free_pkginfo(tool);
    }

    array_str_free(&tool->managed_pkgs);

    memset((void *)tool, 0, sizeof(Tool));
}

