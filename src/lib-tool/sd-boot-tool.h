// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-tool.h
 */
#ifndef SD_BOOT_TOOL_H
#define SD_BOOT_TOOL_H

#include "sd-boot-config.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"

int init_oper(char *oper_str, Tool *tool);
int init_pkginfos_all_managed(Tool *tool);
int init_pkginfos_from_pkg_name(char *pkg_name, Tool *tool);

int initialize_tool(ToolType tool_type, int num_argc, int argc, char *argv[], Tool *tool);

int read_managed_packages(SdBoot *conf, Array_str *arr);
int load_managed_package_list(Tool *tool);

int pkginfos_from_pkg_name(char *pkg_name, Tool *tool);
int pkginfos_update_from_pkg_name(Tool *tool);
int pkginfos_from_triggers(TriggerInfo *triggers, Tool *tool);

#endif
