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
 * Update tools are given work to do.
 * conf include is_kernel, is_efi_tool, is_uki
 * mod_dir - only used by kernel installers.
 *
 * pkg_name, pkg_vers: package name and version
 * ki_vers: the kernel version used by kernel-install
 */
typedef struct PkgInfo {
    char *pkg_name;
    char *pkg_vers;

    char *vers_curr;
    char *vers_prev;

    char *ki_vers;
    char *ki_image;

    char *mod_dir;

    bool managed;

} PkgInfo;

typedef struct Tool {
    SdBoot conf;

    size_t num_pkgs;
    PkgInfo *pkginfo;

    bool managed;
    bool triggers;

    Array_str managed_pkgs;

} Tool;

/*
 * Triggers:
 * Direct read of stdin:
 *   - pkgs
 *   - paths
 *
 * Parsed view (which can steal pointers from above):
 *   - pkgs_special
 *   - pkgs_other
 *   - paths_special
 *   - paths_other
 * Which packages/paths are considered useful is 
 * done by lib-efi and lib-kernel for their tool type
 */
typedef struct TriggerInfo {
    Array_str pkgs;
    Array_str paths;

    Array_str pkgs_special;
    Array_str pkgs_other;

    Array_str paths_special;
    Array_str paths_other;

} TriggerInfo;

/*
 * Function declarations
 */

bool check_permission(SdBoot *conf);
char *read_efi_var_string(const char *efi_path);
int get_triggers(TriggerInfo *trigs);

void print_ki_plugin(KIplugin *plugin);
int plugin_init(int argc, const char *argv[], KIplugin *plugin);
void plugin_free(KIplugin *plugin);

int ki_plugin_env_init(SdBoot *conf);

int kernel_install_run(SdBoot *conf, Array_str *argp, Array_str *envp);

int loaderentry_modify_file(LoaderEntry *entry);
int make_kernel_install_conf(KInstallMods *mods, const char *src, const char *dst);
void pkginfo_free(PkgInfo *info);

int get_active_plugins(SdBoot *conf);
int get_all_plugins(SdBoot *conf);

int read_triggers(Array_str *arr);

int tool_alloc(size_t num_pkgs, Tool *tool);
void tool_free(Tool *tool);

bool trigger_is_path(const char *trigger);

void triggerinfo_free(TriggerInfo *trig);

int ukify_os_release(const char *name, const char *build_id, Dynamic_str *os_release);

#endif
