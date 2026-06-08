// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Path to yaml or toml sd-boot config files 
 *
 * Buffer passed in must be of length PATH_MAX
 */
#include <linux/limits.h>
#include <stdio.h>

#include "sd-boot.h"

int yaml_config_path(SdBoot *conf, char *path) {

    if (!path || !conf) {
        return -1;
    }

    if (snprintf(path, PATH_MAX, "%s%s", conf->info.root, "etc/sd-boot/config.yaml") < 0) {
        perror(nullptr);
        return -1;
    }

    return 0;
}

int yaml_config_sample_path(SdBoot *conf, char *path) {

    if (!path || !conf) {
        return -1;
    }

    if (snprintf(path, PATH_MAX, "%s%s", conf->info.root, "usr/share/sd-boot/config.yaml.sample") < 0) {
        perror(nullptr);
        return -1;
    }

    return 0;
}

int toml_config_path(SdBoot *conf, char *path) {

    if (!path || !conf) {
        return -1;
    }

    if (snprintf(path, PATH_MAX, "%s%s", conf->info.root, "etc/sd-boot/config") < 0) {
        perror(nullptr);
        return -1;
    }

    return 0;
}


