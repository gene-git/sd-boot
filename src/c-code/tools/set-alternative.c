// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Set alternative to one of:
 * - binary 
 * - bash
 *
 * Update the symlinks in /etc/kernel/install.d and /usr/lib/sd-boot
 * to symlink to point to appropriate binary or bash code:
 *
 * NB: There is no "alternative" bash version of set-alternative.
 * NB: Supports SDB_DEV_TEST BOOT_ROOT so can be used to set alternative in the
 *     testing tree.
 */
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

enum Alternative {
    UNKNOWN = 0, 
    BINARY = 1, 
    BASH = 2,
};

enum Task {
    ADD_LINK = 1,
    DEL_LINK = 2,
};

/*
 * Check choice passed in
 */
static enum Alternative get_choice(int argc, const char *argv[]) {

    if (argc < 2) {
        msg(MSG_ERR, "sd-boot: Missing sd-boot alternative\n");
        msg(MSG_ERR, "         One of binary or bash\n");
        return UNKNOWN;
    }
    const char *choice = argv[1];

    if (strcmp(choice, "binary") == 0) {
        return BINARY;
    }

    if (strcmp(choice, "bash") == 0) {
        return BASH;
    }

    return UNKNOWN;
}

static const char *which_root(int argc, const char *argv[], SdBoot *conf) {

    if (argc >= 3) {
        return argv[2];
    }
    return conf->info.root;
} 

static int add_del(enum Task task, const char *root, const char *dir, const char *alt, const char *file) {
    /*
     * <root>/<dir>/<file> -> <alt>/<file>
     */
    int ret = 0;
    char linkname[PATH_MAX] = {'\0'};
    char target[PATH_MAX] = {'\0'};

    if (snprintf(linkname, PATH_MAX, "%s/%s/%s", root, dir, file) < 0) {
        msg(MSG_ERR, "  sd-boot error creating link name for: %s/%s/%s\n", root, dir, file);
        ret = 1;
        goto exit;
    }
    if (snprintf(target, PATH_MAX, "%s/%s", alt, file) < 0) {
        msg(MSG_ERR, "  sd-boot error creating link target name for: %s/%s\n", alt, file);
        ret = 1;
        goto exit;
    }

    switch (task) {
        case ADD_LINK:
            ret = make_symlink(target, linkname);
            if (ret != 0) {
                ret = 1;
                goto exit;
            }
            break;

        case DEL_LINK:
            ret = remove_file(linkname);
            if (ret < 0) {
                ret = 1;
                goto exit;
            }
            break;

        default:
            break;
    }

exit:
    return ret;
}

static int set_alternative_binary(enum Task task, const char *root){
    /*
     * /etc/kernel/install.d
     */
    int ret = 0;
    const char *alt = "binary";
    const char *plugins_dir = "etc/kernel/install.d";
    const char *plugins[] = {
        "95-sd-boot-loaderentry-modify.install",
    };
    size_t num_plugins = sizeof(plugins) / sizeof(plugins[0]);
    const char *tools_dir = "usr/lib/sd-boot";
    const char *tools[] = {
        "sd-boot-efifs-update",
        "sd-boot-efi-tool-update",
        "sd-boot-find-boot-mounts",
        "sd-boot-kernel-update",
    };
    size_t num_tools = sizeof(tools) / sizeof(tools[0]);

    for (size_t i = 0; i < num_plugins; i++) {
        ret = add_del(task, root, plugins_dir, alt, plugins[i]);
        if (ret != 0) {
            goto exit;
        }
    }

    for (size_t i = 0; i < num_tools; i++) {
        ret = add_del(task, root, tools_dir, alt, tools[i]);
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    return ret;
}

static int set_alternative_bash(enum Task task, const char *root){
    /*
     * /etc/kernel/install.d
     */
    int ret = 0;
    const char *alt = "bash";
    const char *plugins_dir = "etc/kernel/install.d";
    const char *plugins[] = {
        "95-sd-boot-loaderentry-efi.install",
        "95-sd-boot-loaderentry-kernel.install",
    };
    size_t num_plugins = sizeof(plugins) / sizeof(plugins[0]);
    const char *tools_dir = "usr/lib/sd-boot";
    const char *tools[] = {
        "sd-boot-efifs-update",
        "sd-boot-efi-tool-update",
        "sd-boot-find-boot-mounts",
        "sd-boot-kernel-update",
    };
    size_t num_tools = sizeof(tools) / sizeof(tools[0]);

    for (size_t i = 0; i < num_plugins; i++) {
        ret = add_del(task, root, plugins_dir, alt, plugins[i]);
        if (ret != 0) {
            goto exit;
        }
    }

    for (size_t i = 0; i < num_tools; i++) {
        ret = add_del(task, root, tools_dir, alt, tools[i]);
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    return ret;
}


/**
 * set-alternative <choice> [<root>]
 * - <choice> is "binary" or "bash"
 * - if <root> specified it is used. 
 *   If not the testing "root" triggered by SDB_DEV_TEST
 *   If not then "/"
 *
 * The PKGBUILD uses <root> option to set the default alternative into $pkgdir
 */
int main(int argc, const char *argv[]) {
    int ret = 0;
    SdBoot conf = {};
    const enum Alternative choice = get_choice(argc, argv);

    if (choice == UNKNOWN) {
        ret = 1;
        goto exit;
    }

    /*
     * conf provides any root dir used in testing.
     */
    (void) load_config(&conf);
    const char *root = which_root(argc, argv, &conf);

    switch (choice) {
        case BINARY:
            ret = set_alternative_bash(DEL_LINK, root);
            if (ret != 0) {
                break;
            }
            ret = set_alternative_binary(ADD_LINK, root);
            if (ret != 0) {
                break;
            }
            break;

        case BASH:
            ret = set_alternative_binary(DEL_LINK, root);
            if (ret != 0) {
                break;
            }
            ret = set_alternative_bash(ADD_LINK, root);
            if (ret != 0) {
                break;
            }
            break;

        default:
            ret = 1;
            msg(MSG_ERR, "  sd-boot - unknown alterative : %s\n", choice);
    }

exit:
    clean_config(&conf);
    return ret;
}
