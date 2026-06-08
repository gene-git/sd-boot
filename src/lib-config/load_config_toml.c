// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * read (toml format) config file
 * - /etc/sd-boot/config
 *
 * Contents  (comments ignored - line starting with #)
 * verb = <integer>
 */
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

enum Constants { VERB_MAX = 2 };

/*
 * Load /etc/sd-boot/config
 * Contains one element:
 * - verb
 *
 * Return:
 * -1 = error
 *  0 = all good
 *  1 = config file not available or empty.
 */
int load_config_toml(SdBoot *conf) {
    char path[PATH_MAX] = {};
    int ret = 0;
    KvElem *elems = nullptr;
    size_t num_elems = 0;

    /*
     * File to read
     */
    ret = toml_config_path(conf, path);
    if (ret != 0) {
        goto exit;
    }

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
    if (ret == 1) {
        ret = 0;
    }
     */

exit:
    if (elems) {
        free((void *)elems);
    }
    return ret;
}

