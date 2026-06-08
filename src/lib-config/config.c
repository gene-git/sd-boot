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

#include "sd-boot.h"

enum Constants { VERB_MAX = 2 };

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

int load_config(SdBoot *conf) {
    int ret = 0;
    struct ConfigFiles file_info = {};

    /*
     * load development info
     * - env SDB_DEV_TEST activated dev / testing 
     *   using a separaate root tree (non-root)
     * Initializes conf->info.root used below.
     */
    if (init_devinfo(&conf->info) != 0) {
        return -1;
    }

    /*
     * Load sd-boot config
     * - try yaml first 
     * - if no yaml, read toml (if available) and convert to yaml.
     *   Conversion errors are non-fatal.
     */
    ret = config_file_info(conf, &file_info);
    if (ret != 0) {
        goto exit;
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
         * toml - convert to yaml
         */
        if (load_config_toml(conf) == 0) { 
            convert_config(conf);
        }
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

