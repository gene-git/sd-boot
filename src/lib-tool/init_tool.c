// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Initialize Tool
 *
 * conf must be loaded and conf->is_kernel / conf->is_efi_tool should be set.
 * - Parse command line arguments
 * - Set operation : conf->oper
 * - Save package info - one named package or all packages of same type:
 *   Command line can contain a single package name or --all-packages--
 * - For each package pkginfo is updated with the package:
 *   - name
 *   - version
 *   - image
 *   - mod_dir
 *   - is_sd_boot_managed
 *
 */
#include <stddef.h>
#include <stdlib.h>

#include "sd-boot-msg.h"
#include "sd-boot-config.h"
#include "sd-boot-tool.h"
#include "sd-boot.h"


/*
 * Initialize:
 *
 * - Read config file (tool->conf)
 * - parse command line and fill pkg details out tool->pkginfo)
 *
 *  Oper is one of the supported operations (add, remove, inspect).
 *  package is either a package name or "--all--" which returns all relevant packages
 *
 *  Input:
 *      tool_type: kernel or efi-tool or ...
 *      num_args: expected number of argc.
 *      argc, argv: command line args must be:
 *                  <oper> <package(s)>
 *  Output:
 *   tool->conf
 *   tool->pkginfo
 *
 *  Returns 0 when all is fine.
 */
int initialize_tool(ToolType tool_type, int num_argc, int argc, char *argv[], Tool *tool) {
    int ret = 0;
    char *oper = nullptr;
    TriggerInfo trigger_info = {};

    /*
     * Opertion (add/inspect/remove)
     */
    if (argc < num_argc) {
        ret = 1;
        goto exit;
    }

    /*
     * load config
     * - also sets verbosity level
     */
    if (load_config(&tool->conf) != 0) {
        msg(MSG_ERR, "- sd-boot: warning failed to load config file\n");
    }
    tool->conf.tool_type = tool_type;

    /*
     * Get list of managed packages
     */
    ret = load_managed_package_list(tool);
    if (ret != 0) {
        goto exit;
    }

    /*
     * perms
     */
    if (!check_permission(&tool->conf)) {
        ret = 1;
        goto exit;
    }

    /*
     * <oper>
     */
    oper = argv[1];
    ret = init_oper(oper, tool);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Initialize pkginfos from either:
     * - triggers 
     * or
     * - package name
     *   And name "--all--" means all packages of same tool type
     */
    if (tool->triggers) {

        ret = get_triggers(&trigger_info);
        if (ret != 0) {
            goto exit;
        }

        ret = pkginfos_from_triggers(&trigger_info, tool);

    } else {

        char *pkg_name = argv[2];

        ret = pkginfos_from_pkg_name(pkg_name, tool);

    }
    if (ret != 0) {
        goto exit;
    }

    /*
     * Manaed true if any pkgs are managed by sd-boot
     */
    tool->managed = false;
    for (size_t i = 0; i < tool->num_pkgs; i++) {
        if (tool->pkginfo[i].managed) {
            tool->managed = true;
            break;
        }
    }

exit:
    triggerinfo_free(&trigger_info);
    return ret;
}
