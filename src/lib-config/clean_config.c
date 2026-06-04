// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Free memory in SdBoot
 */
#include <stdlib.h>

#include "sd-boot.h"

void clean_config(SdBoot *conf) {
    clean_devinfo(&conf->info);

    array_str_free(&conf->skip_kernel_plugins);
    
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
}
