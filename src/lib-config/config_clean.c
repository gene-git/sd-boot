// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Free memory in SdBoot
 */
#include <stdlib.h>
#include <string.h>

#include "sd-boot-config.h"
#include "sd-boot-utils.h"

void config_clean(SdBoot *conf) {

    array_str_free(&conf->skip_kernel_plugins);
    
    if (conf->root) {
        free((void *)conf->root);
    }

    if (conf->test_root) {
        free((void *)conf->test_root);
    }

    if (conf->layout) {
        free((void *)conf->layout);
    }

    if (conf->initrd_generator) {
        free((void *)conf->initrd_generator);
    }

    if (conf->uki_generator) {
        free((void *)conf->uki_generator);
    }

    if (conf->oper_str) {
        free((void *)conf->oper_str);
    }

    if (conf->kernel_conf_dir) {
        free((void *)conf->kernel_conf_dir);
    }

    if (conf->kernel_conf_bls_dir) {
        free((void *)conf->kernel_conf_bls_dir);
    }

    array_str_free(&conf->all_plugins);
    array_str_free(&conf->active_plugins);

    array_str_free(&conf->env_boot_root);
    array_str_free(&conf->env_active_plugins);
    array_str_free(&conf->env_base);

    memset((void *)conf, 0, sizeof(SdBoot));

}
