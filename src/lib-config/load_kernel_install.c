// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * read and parse kernel-install "install.conf" config files.
 * See man kernel-install and below for more detail on "install.conf" 
 * and the companion drop in files are read.
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-utils.h"
#include "sd-boot-config.h"

/*
 * Check BOOT_ROOT and MACHINE_ID
 */
static int load_env_vars(SdBoot *conf) {
    char *env_var = nullptr;

    env_var = secure_getenv("BOOT-ROOT");
    if (env_var) {
        conf->install_conf.env_var_boot_root = strdup(env_var);
        if (!conf->install_conf.env_var_boot_root) {
            return -1;
        }
    }

    env_var = secure_getenv("MACHINE-ID");
    if (env_var) {
        conf->install_conf.env_var_machine_id = strdup(env_var);
        if (!conf->install_conf.env_var_machine_id ) {
            return -1;
        }
    }

    return 0;
}

/*
 * Read /etc/kernel/entry-token and /usr/lib/machine-id
 */
static void read_etc_files(SdBoot *conf) {
    const char *f_entry_token = "/etc/kernel/entry-token";
    const char *f_machine_id = "/etc/machine-id";
    InstallConf *iconf = &conf->install_conf;

    iconf->entry_token_etc = read_file_first_row(f_entry_token);
    iconf->machine_id_etc = read_file_first_row(f_machine_id);
}

int load_kernel_install_conf(SdBoot *conf) {

    if (!conf) {
        return -1;
    }
    InstallConf *iconf = &conf->install_conf;

    /*
     * - read the install.conf and their drop in files (conf->install_conf)
     * - checkk env BOOT_ROOT and MACHINE_ID
     */
    if (read_kernel_install_files(conf) != 0) {
        return -1;
    }

    /*
     * /etc/kernel/entry-token
     * /usr/lib/machine-id
     */
    read_etc_files(conf);

    /*
     * Envs
     */
    if (load_env_vars(conf) != 0) {
        return -1;
    }

    /*
     * Dertermine values to use.
     */

    /*
     * layout
     */
    if (iconf->layout) {
        conf->layout = strdup(iconf->layout);
    } else {
        conf->layout = strdup("uki");
    }
    if (!conf->layout) {
        return -1;
    }

    /*
     * initrd_generator
     */
    if (iconf->initrd_generator) {
        conf->initrd_generator = strdup(iconf->initrd_generator);
    } else {
        conf->initrd_generator = strdup("dracut");
    }
    if (!conf->initrd_generator) {
        return -1;
    }

    /*
     * uki_generator
     */
    if (iconf->uki_generator) {
        conf->uki_generator = strdup(iconf->uki_generator);
    } else {
        conf->uki_generator = strdup("ukify");
    }
    if (!conf->uki_generator) {
        return -1;
    }

    /*
     * entry_token
     */
    if (iconf->entry_token_etc) {
        conf->entry_token = strdup(iconf->entry_token_etc);

    } else if (iconf->env_var_machine_id) {
        conf->entry_token = strdup(iconf->env_var_machine_id);

    } else if (iconf->machine_id_etc) {
        conf->entry_token = strdup(iconf->machine_id_etc);

    } else {
        conf->entry_token = strdup("");
    }
    if (!conf->entry_token) {
        return -1;
    }

    /*
     * boot_root - leave as null if not set
     */
    if (iconf->boot_root) {
        conf->boot_root = strdup(iconf->boot_root);
        if (!conf->boot_root) {
            return -1;
        }
    }

    return 0;
}

