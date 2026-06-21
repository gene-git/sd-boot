// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * read and parse config file
 * - /etc/kernel/install.conf
 *
 * Contents can only be 
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

#include "sd-boot-msg.h"
#include "sd-boot-config.h"
#include "sd-boot-utils.h"

enum Constants { VERB_MAX = 2 };

/*
 * Load <root>/etc/kernel/install.conf
 *
 * Contains up to 3 element(s):
 * - layout             bls, uki 
 * - initrd_generator   dracut, mkiitcpio
 * - uki_generator      ukify
 *
 * Content are comments or "key = value"
 */
int load_kernel_install_conf(SdBoot *conf) {
    char path[PATH_MAX] = {};
    int ret = 0;
    KvElem *elems = nullptr;
    size_t num_elems = 0;

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

    if (snprintf(path, sizeof(path), "%s%s", conf->root, "etc/kernel/install.conf") < 0) {
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
    if (elems) {
        free((void *)elems);
    }
    return ret;
}


