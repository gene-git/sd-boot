// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * read config files
 * - /etc/sd-boot/config
 * - /etc/kernel/install.conf
 *
 * Contents can be 
 * - key = value pairs
 * - skips comments (# ...)
 */
#include <limits.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <unistd.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"
#include "sd-boot.h"

/**
 * read config files
 * - /etc/sd-boot/config.yaml or /etc/sd-boot/config
 * - /etc/kernel/install.conf
 *
 * Config management:
 * - if have config.yaml => use it
 * - else if have config => convert to config.yaml
 * - else copy sample to config.yaml.
 *
 * Return:
 *   0 = success (includes no config files)
 *  -1 = failed to load (warning) 
 *  -2 = fatal (root required)
 */

struct ConfigFiles {
    bool have_yaml;
    char yaml_file[PATH_MAX];

    bool have_toml;
    char toml_file[PATH_MAX];

    char yaml_sample[PATH_MAX];
};

static int config_file_info(SdBoot *conf, struct ConfigFiles *file_info) {
    int ret = 0;

    ret = yaml_config_path(conf, file_info->yaml_file);
    if (ret != 0) {
        goto exit;
    }
    file_info->have_yaml = (bool)(access(file_info->yaml_file, F_OK) == 0);

    ret = toml_config_path(conf, file_info->toml_file);
    if (ret != 0) {
        goto exit;
    }
    file_info->have_toml = (bool)(access(file_info->toml_file, F_OK) == 0);

    ret = yaml_config_sample_path(conf, file_info->yaml_sample);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}

/*
 * Read sd-boot config
 * - try yaml first 
 * - if no yaml, read toml (if available) and then convert to yaml.
 *   Conversion errors are non-fatal.
 */
static void read_config_file(SdBoot *conf) {

    struct ConfigFiles file_info = {};

    if (config_file_info(conf, &file_info) != 0) {
        return;
    }

    if (!file_info.have_yaml && !file_info.have_toml) {
        /*
         * No configs - copy sample
         */
        (void)copy_file(file_info.yaml_sample, file_info.yaml_file);

    } else if (file_info.have_yaml) {
        /*
         * yaml
         */
        (void)load_config_yaml(conf);

    } else if (file_info.have_toml) {
        /*
         * (Old) toml - convert to yaml
         */
        if (load_config_toml(conf) == 0) { 
            convert_config(conf);
        }
    }
}

int load_config(SdBoot *conf) {
    int ret = 0;

    /*
     * Initialize conf->root, test mode (SDB_DEV_TEST) etc
     * - also sets env variable BOOT_ROOT in conf->env_boot_root
     */
    if (config_init(conf) != 0) {
        return -1;
    }

    /*
     * Load sd-boot config
     */
    read_config_file(conf);

    /*
     * Check availibility
     */
    // conf->unshare_available = unshare_available();
    conf->efivars_available = efivars_available();

    /*
     * Plugins:
     * update 
     *   - conf->all_plugins
     *   - conf->active_plugins
     * - must do after config is read since it has list of plugins to skip.
     */
    ret = get_all_plugins(conf);
    if (ret != 0) {
        goto exit;
    }

    ret = get_active_plugins(conf);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Environment variables
     * - KERNEL_INSTALL_PLUGINS: set to the list of active plugins
     * - limitied PATH
     */
    ret = ki_plugin_env_init(conf);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Env var:
     * - PATH
     * - for test pass additional variables. e.g.
     *   LD_LIBRARY_PATH for running tests in non-production)
     */
    ret = config_set_base_env(conf->test, &conf->env_base);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Initialize the dirs used for shadow "bls" kernel-install install.conf
     */
    ret = ki_install_conf_init(conf);
    if (ret < 0) {
        goto exit;
    }

    /*
     * load info from the normal install.conf
     */
    ret = load_kernel_install_conf(conf);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}

