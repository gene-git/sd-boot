// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * read config file
 * Contents can be 
 * - comments (# ...)
 * - key = value pairs
 */
#include <limits.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "sd-boot.h"

enum Constants { VERB_MAX = 2 };


int load_config(SdBoot *conf) {
    char path[PATH_MAX] = {'\0'};
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
     * Load the config preferences 
     * Contains the elements:
     * - verb
     */
    conf->num = 1;
    ret = alloc_kv_elems(conf->num, &conf->elem);
    if (ret != 0) {
        msg(MSG_ERR, " sd-boot: load config got mem allocation error\n");
        goto exit;
    }

    conf->elem[0].key = "verb";
    conf->elem[0].type = CONF_INT;
    conf->elem[0].lo_val_int = 0;
    conf->elem[0].hi_val_int = VERB_MAX;

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
    ret = read_kv_elems(path, conf->num, conf->elem, &num_elems_read);
    if (ret == 0 && num_elems_read > 0) {
        // set verbosity level for msg()
        set_verb_level(conf->elem[0].val.v_int);
    }

exit:
    return ret;
}

void clean_config(SdBoot *conf) {
    clean_devinfo(&conf->info);
    
    if (conf->elem != nullptr) {
        free((void *)conf->elem);
    }
}
