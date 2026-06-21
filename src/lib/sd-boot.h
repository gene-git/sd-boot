// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot.h
 */
#ifndef SD_BOOT_LIB_H
#define SD_BOOT_LIB_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"

/*
 *
 * Kernel-Install Plugin
 */
typedef struct {
    // args 
    char *command;
    char *kernel_version;
    char *entry_dir_abs;
    char *kernel_image;

    char *initrd_1;
    char *initrd_2;

    // environ
    char *layout;
    char *verbose;
    char *machine_id;
    char *entry_token;
    char *boot_root;
    bool is_uki;

    char *loader_entry_dir;
    char *loader_entry_file;

    // kernel or efi tool
    bool is_kernel;
    bool is_efi_tool;

} KIplugin;

// How to modify a loader entry
typedef struct {
    bool is_efi_tool;
    bool is_kernel;

    char *loader_entry_dir;
    char *loader_entry_file;
    char *title;
} LoaderEntry;

/*
 * Modifications for "shadow" bls (kernel) install.conf
 */
typedef struct KInstallMods {
    char *layout;
    char *initrd_generator;
    char *uki_generator;
} KInstallMods;

/*
 * Function declarations
 */

bool check_permission(SdBoot *conf);
char *read_efi_var_string(const char *efi_path);

int get_active_plugins(SdBoot *conf);
int get_all_plugins(SdBoot *conf);

int plugin_init(int argc, const char *argv[], KIplugin *plugin);
void plugin_free(KIplugin *plugin);
int ki_plugins_test_env(char *test_root, Array_str *env);
int ki_plugin_env_init(SdBoot *conf);

int kernel_install_run(SdBoot *conf, Array_str *argp, Array_str *envp);
int loaderentry_modify_file(LoaderEntry *entry);
int make_kernel_install_conf(KInstallMods *mods, const char *src, const char *dst);

int read_triggers(Array_str *arr);
int ukify_os_release(const char *name, const char *build_id, Dynamic_str *os_release);

#endif
