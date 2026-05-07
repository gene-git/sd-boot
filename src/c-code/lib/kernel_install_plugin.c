// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 *
 * Kernel Install Plugins
 *
 * plugins called from kernel-install get passed information via command line and
 * via environment variables
 *
 * Command line in order:
 * - COMMAND
 * - KERNEL_VERSION
 * - ENTRY_DIR_ABS
 * - KERNEL_IMAGE
 * - INITRD_1 INITRD_2 ..
 *
 * Env variables:
 * KERNEL_INSTALL_LAYOUT
 * KERNEL_INSTALL_VERBOSE
 * KERNEL_INSTALL_MACHINE_ID
 * KERNEL_INSTALL_ENTRY_TOKEN
 * KERNEL_INSTALL_BOOT_ROOT
 *
 * =>
 * loader_entries_dir = BOOT_ROOT/loader/entries
 * loader_entry_file = ENTRY_TOKEN-KERNEL_VERSION.conf
 *
 * plugins return 0 = success
 *               77 = terminate the entire chain
 *
 */
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

enum Constants {
    MAX_STR = PATH_MAX,
    CMD_1 = 1,
    CMD_2 = 2,
    CMD_3 = 3,
    CMD_4 = 4,
    CMD_5 = 5,
    CMD_6 = 6,

};


static int copy_argv_item(int idx, int argc, const char **argv, char **ptr) {
    /*
     * makes a copy of argv[idx]
     * - empty string is ok
     * kernel-install passes down based on how it is called
     * So we treat a missing item as empty string.
     */
    if (argv == nullptr) {
        return -1;
    }

    char *str = nullptr;
    if (idx > argc || argv[idx] == nullptr) { 
        str = "";
    } else {
        str = (char *)argv[idx];
    }
    size_t len = strlen(str);
    *ptr = strndup((const char *)str, len);
    return 0;
}

static int copy_env_item(char *name, char **ptr) {
    /*
     * makes a copy of env variable "name"
     * - empty string is ok
     */
    if (name == nullptr) {
        return -1;
    }

    char *env = nullptr;
    env = secure_getenv(name);
    if (env == nullptr) {
        env = "";
    }
    *ptr = strndup(env, MAX_STR);
    return 0;
}

static int loader_entry(KIplugin *plugin) {
    /*
     * Path to loader entry file
     */
    int ret = 0;
    char buf[PATH_MAX] = {'\0'};

    /*
     * loader entry dir
     */
    if (snprintf(buf, PATH_MAX, "%s/loader/entries", plugin->boot_root) < 0) {
        ret = -1;
    }
    plugin->loader_entry_dir = strndup(buf, PATH_MAX);

    /*
     * loader entry filename
     */
    if (snprintf(buf, PATH_MAX, "%s-%s.conf", plugin->entry_token, plugin->kernel_version) < 0) {
        ret = -1;
    }
    plugin->loader_entry_file = strndup(buf, PATH_MAX);

    return ret;
}

void print_ki_plugin(KIplugin *plugin) {
    msg(MSG_ERR, "print_ki_plugin\n");
    if (plugin->command) {
        msg(MSG_ERR, "%20s : %s\n", "command", plugin->command);
    }
    if (plugin->kernel_version) {
        msg(MSG_ERR, "%20s : %s\n", "kernel_version", plugin->kernel_version);
    }
    if (plugin->entry_dir_abs) {
        msg(MSG_ERR, "%20s : %s\n", "entry_dir_abs", plugin->entry_dir_abs);
    }
    if (plugin->kernel_image) {
        msg(MSG_ERR, "%20s : %s\n", "kernel_image", plugin->kernel_image);
    }
    if (plugin->initrd_1) {
        msg(MSG_ERR, "%20s : %s\n", "initrd_1", plugin->initrd_1);
    }
    if (plugin->initrd_2) {
        msg(MSG_ERR, "%20s : %s\n", "initrd_2", plugin->initrd_2);
    }
    if (plugin->layout) {
        msg(MSG_ERR, "%20s : %s\n", "layout", plugin->layout);
    }
    if (plugin->verbose) {
        msg(MSG_ERR, "%20s : %s\n", "verbose", plugin->verbose);
    }
    if (plugin->machine_id) {
        msg(MSG_ERR, "%20s : %s\n", "machine_id", plugin->machine_id);
    }
    if (plugin->entry_token) {
        msg(MSG_ERR, "%20s : %s\n", "entry_token", plugin->entry_token);
    }
    if (plugin->boot_root) {
        msg(MSG_ERR, "%20s : %s\n", "boot_root", plugin->boot_root);
    }
    if (plugin->loader_entry_dir) {
        msg(MSG_ERR, "%20s : %s\n", "loader_entry_dir", plugin->loader_entry_dir);
    }
    if (plugin->loader_entry_file) {
        msg(MSG_ERR, "%20s : %s\n", "loader_entry_file", plugin->loader_entry_file);
    }
}

int plugin_init(int argc, const char *argv[], KIplugin *plugin) {
    /*
     * @plugin_init Initialize information passed to a plugin by kernel-install
     * @plugin Data to initilize (caller must free with plugin_free()
     *
     * NB For simplicity we limit command line initrd to 2 entries.
     */
    int ret = 0;

    /*
     * command line 
     */
    ret = copy_argv_item(CMD_1, argc, argv, &plugin->command);
    if (ret != 0) {
        goto exit;
    }

    ret = copy_argv_item(CMD_2, argc, argv, &plugin->kernel_version);
    if (ret != 0) {
        goto exit;
    }

    ret = copy_argv_item(CMD_3, argc, argv, &plugin->entry_dir_abs);
    if (ret != 0) {
        goto exit;
    }

    ret = copy_argv_item(CMD_4, argc, argv, &plugin->kernel_image);
    if (ret != 0) {
        goto exit;
    }

    ret = copy_argv_item(CMD_5, argc, argv, &plugin->initrd_1);
    if (ret != 0) {
        goto exit;
    }

    ret = copy_argv_item(CMD_6, argc, argv, &plugin->initrd_2);
    if (ret != 0) {
        goto exit;
    }

    /*
     * env 
     */
    ret = copy_env_item("KERNEL_INSTALL_LAYOUT", &plugin->layout);
    if (ret != 0) {
        goto exit;
    }

    ret = copy_env_item("KERNEL_INSTALL_VERBOSE", &plugin->verbose);
    if (ret != 0) {
        goto exit;
    }

    ret = copy_env_item("KERNEL_INSTALL_MACHINE_ID", &plugin->machine_id);
    if (ret != 0) {
        goto exit;
    }

    ret = copy_env_item("KERNEL_INSTALL_ENTRY_TOKEN", &plugin->entry_token);
    if (ret != 0) {
        goto exit;
    }

    ret = copy_env_item("KERNEL_INSTALL_BOOT_ROOT", &plugin->boot_root);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Loader entry info
     */
    ret = loader_entry(plugin);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Determine if the image is a kernel source 
     */
    ret = is_kernel_image_path(plugin->kernel_image, &plugin->is_kernel);
    
    /*
     * dev debug
    print_ki_plugin(plugin);
     */

exit:
    return ret;
}

void plugin_free(KIplugin *plugin) {
    /*
     * free up any non-null mem
     */
    if (plugin->command != nullptr) {
        free((void *) plugin->command);
    }

    if (plugin->kernel_version != nullptr) {
        free((void *) plugin->kernel_version);
    }

    if (plugin->entry_dir_abs != nullptr) {
        free((void *) plugin->entry_dir_abs);
    }

    if (plugin->kernel_image != nullptr) {
        free((void *) plugin->kernel_image);
    }

    if (plugin->initrd_1 != nullptr) {
        free((void *) plugin->initrd_1);
    }

    if (plugin->initrd_2 != nullptr) {
        free((void *) plugin->initrd_2);
    }

    if (plugin->layout != nullptr) {
        free((void *) plugin->layout);
    }

    if (plugin->verbose != nullptr) {
        free((void *) plugin->verbose);
    }

    if (plugin->machine_id != nullptr) {
        free((void *) plugin->machine_id);
    }

    if (plugin->entry_token != nullptr) {
        free((void *) plugin->entry_token);
    }

    if (plugin->boot_root != nullptr) {
        free((void *) plugin->boot_root);
    }

    if (plugin->loader_entry_dir != nullptr) {
        free((void *) plugin->loader_entry_dir);
    }

    if (plugin->loader_entry_file != nullptr) {
        free((void *) plugin->loader_entry_file);
    }
}


