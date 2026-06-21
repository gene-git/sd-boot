// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot.h
 */
#ifndef SD_BOOT_CONFIG_H
#define SD_BOOT_CONFIG_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include "sd-boot-utils.h"

/*
 *
 * Command line kernel-install operations
 */
typedef enum KernelInstallOper {
    KI_BAD = 0,
    KI_ADD = 1,
    KI_REMOVE = 2,
    KI_INSPECT = 3,
    KI_ADD_ALL = 4,
    KI_LIST = 5,
} KernelInstallOper;

/*
 * Data taken from
 * - /etc/sd-boot/config.yaml (or config)
 *   - verb
 *   - str skip_kernel_plugins (list of paths of kernel plugins to skip)
 *
 * - /etc/kernel/install.conf
 *   - layout
 *   - initrd_generator
 *   - uki_generator;
 */
typedef struct {

    int verb;
    Array_str skip_kernel_plugins;

    bool test;
    uid_t euid;
    char *root;

    bool is_uki;
    char *layout;
    char *initrd_generator;
    char *uki_generator;
    bool is_efi_tool;

    KernelInstallOper oper;
    char *oper_str;

    char *kernel_conf_dir;
    char *kernel_conf_bls_dir;

    /*
     * Plugin lists
     */
    Array_str all_plugins;
    Array_str active_plugins;

    /*
     * Testing
     * - when available root = "/"
     * - test_root = "/path/to/test/tree/root"
     * - build (which runs tests) may be inside chroot where:
     *   - /sys/firmware/efi/efivars may not be available.
     *   - unshare may not be available.
     */
    char *test_root;
    bool test_with_chroot;
    bool unshare_available;
    bool efivars_available;

    /*
     * Env variables BOOT_ROOT, KERNEL_INSTALL_PLUGINS
     * Used by kernel-install to install kernels and efi-tools
     * - env_active_plugins;  generated from active plugin list.
     *   this is only needed if either:
     *   - test mode is active (so it uses the plugins from test tree
     *   - skip_kernel_plugins is set to limit which plugins are used.
     *   - for simnplicity the env to set list of active plugins for kernel-install
     *     is always created.
     *
     * Kernels use:
     *  - env_boot_root
     *  - env_active_plugins
     *
     * efi-tools:
     *  - env_boot_root
     *  - special list of relevant plugins.
     */
    Array_str env_boot_root;
    Array_str env_active_plugins;
    Array_str env_base;

} SdBoot;

/*
 * Function declarations
 */


/*
 * lib-config
 */
void config_clean(SdBoot *conf);
int load_config(SdBoot *conf);
int toml_config_path(SdBoot *conf, char *path);
int yaml_config_path(SdBoot *conf, char *path);
int yaml_config_sample_path(SdBoot *conf, char *path);
int load_config_toml(SdBoot *conf);
int load_config_yaml(SdBoot *conf);
void convert_config(SdBoot *conf);
KernelInstallOper kernel_install_oper(const char *oper);
int ki_install_conf_init(SdBoot *conf);
int load_kernel_install_conf(SdBoot *conf);
int save_yaml_config(SdBoot *conf, const char *hdr, const char *path);

int config_init(SdBoot *conf);
int config_test_init(SdBoot *conf);

int config_set_base_env(bool test, Array_str *env_base);

#endif
