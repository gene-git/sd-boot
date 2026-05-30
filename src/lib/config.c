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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "sd-boot.h"

enum Constants { VERB_MAX = 2 };

/*
 * Load /etc/sd-boot/config
 * Contains one element:
 * - verb
 */
static int our_config(SdBoot *conf) {
    char path[PATH_MAX] = {'\0'};
    int ret = 0;
    KvElem *elems = nullptr;
    size_t num_elems = 0;

    num_elems = 1;
    ret = alloc_kv_elems(num_elems, &elems);
    if (ret != 0) {
        msg(MSG_ERR, " sd-boot: load config got mem allocation error\n");
        goto exit;
    }

    elems[0].key = "verb";
    elems[0].type = CONF_INT;
    elems[0].lo_val_int = 0;
    elems[0].hi_val_int = VERB_MAX;

    if (snprintf(path, sizeof(path), "%s/%s", conf->info.root, "etc/sd-boot/config") < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    /*
     * Note - read_kv_elems returns:
     * -1 = error
     *  0 = all good
     *  1 = path not found
     */
    size_t num_elems_read = 0;
    ret = read_kv_elems(path, num_elems, elems, &num_elems_read);
    if (ret == 0 && num_elems_read > 0) {
        /*
         * set verbosity level for msg()
         */
        conf->verb = elems[0].val.v_int;
        set_verb_level(conf->verb);
    }

    /* 
     * missing config is not an error 
     */
    if (ret == 1) {
        ret = 0;
    }

exit:
    if (elems != nullptr) {
        free((void *)elems);
    }
    return ret;
}

/*
 * Load /etc/kernel/install.conf
 * Contains up to 3 element(s):
 * - layout             bls, uki 
 * - initrd_generator   dracut, mkiitcpio
 * - uki_generator      ukify
 */
static int kernel_config(SdBoot *conf) {
    char path[PATH_MAX] = {'\0'};
    int ret = 0;
    KvElem *elems = nullptr;
    size_t num_elems = 0;

    /*
     * We dont want kernel_conf_bls_dir configurable
     * its used by efi tool to make a shadow /etc/kernel but with
     * layout set to bls.
     */
    if (snprintf(path, sizeof(path), "%s/%s", conf->info.root, "etc/kernel") < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    conf->kernel_conf_dir = strdup(path);

    if (snprintf(path, sizeof(path), "%s/%s", conf->info.root, "var/lib/sd-boot/kernel_conf_bls") < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    conf->kernel_conf_bls_dir = strdup(path);

    num_elems = 3;
    ret = alloc_kv_elems(num_elems, &elems);
    if (ret != 0) {
        msg(MSG_ERR, " sd-boot: load config got mem allocation error\n");
        goto exit;
    }

    elems[0].key = "layout";
    elems[0].type = CONF_STR;
    elems[0].val.v_str[0] = '\0';

    elems[1].key = "initrd_generator";
    elems[1].type = CONF_STR;
    elems[1].val.v_str[0] = '\0';

    elems[2].key = "uki_generator";
    elems[2].type = CONF_STR;
    elems[2].val.v_str[0] = '\0';

    if (snprintf(path, sizeof(path), "%s/%s", conf->info.root, "etc/kernel/install.conf") < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    /*
     * Note - read_kv_elems returns:
     * -1 = error
     *  0 = all good
     *  1 = path not found
     */
    conf->is_uki = false;
    size_t num_elems_read = 0;
    ret = read_kv_elems(path, num_elems, elems, &num_elems_read);
    if (ret == 0 && num_elems_read > 0) {
        if (elems[0].val.v_str[0] != '\0') {
            conf->layout = strdup(elems[0].val.v_str);
            if (strcasecmp(conf->layout, "uki") == 0) {
                conf->is_uki = true;
            }
        }
        if (elems[1].val.v_str[0] != '\0') {
            conf->initrd_generator = strdup(elems[1].val.v_str);
        }
        if (elems[2].val.v_str[0] != '\0') {
            conf->uki_generator = strdup(elems[2].val.v_str);
        }
    }
    /* 
     * missing config is not an error 
     */
    if (ret == 1) {
        ret = 0;
    }

exit:
    if (elems != nullptr) {
        free((void *)elems);
    }
    return ret;
}

/**
 * read config files
 * - /etc/sd-boot/config
 * - /etc/kernel/install.conf
 */
int load_config(SdBoot *conf) {
    int ret = 0;

    /*
     * load development info
     * - env SDB_DEV_TEST activated dev / testing 
     *   using a separaate root tree (non-root)
     */
    if (init_devinfo(&conf->info) != 0) {
        return -1;
    }

    /*
     * Load our config
     */
    ret = our_config(conf);
    if (ret != 0) {
        goto exit;
    }

    ret = kernel_config(conf);
    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}

void clean_config(SdBoot *conf) {
    clean_devinfo(&conf->info);
    
    if (conf->layout != nullptr) {
        free((void *)conf->layout);
    }
    if (conf->initrd_generator != nullptr) {
        free((void *)conf->initrd_generator);
    }
    if (conf->uki_generator != nullptr) {
        free((void *)conf->uki_generator);
    }
    if (conf->oper_str != nullptr) {
        free((void *)conf->oper_str);
    }
    if (conf->kernel_conf_dir) {
        free((void *)conf->kernel_conf_dir);
    }
    if (conf->kernel_conf_bls_dir) {
        free((void *)conf->kernel_conf_bls_dir);
    }
}
