// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Make a shadow of /etc/kernel in /var/lib/sd-boot/kernel_conf_bls which
 * has symlinks of /etc/kernel/xxx except for install.conf
 *
 * install conf is a copy where layout = bls
 *
 * This is only necessary in the event that the layout is not already bls.
 * but the decision to make the copy is up to the caller.
 */
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

enum Constants {
    ROW_SZ = 1024,
};

static int open_files(SdBoot *conf, FILE **fptr_src, FILE **fptr_dst) {
    int ret = 0;
    const char *file = "install.conf";
    char path[PATH_MAX] = {'\0'};

    *fptr_src = nullptr;
    *fptr_dst = nullptr;
        
    /*
     * Original install.conf
     */
    if (snprintf(path, PATH_MAX, "%s/%s", conf->kernel_conf_dir, file) < 0) {
        ret = -1;
        goto exit;
    }

    *fptr_src = fopen(path, "r");
    if (*fptr_src == nullptr) {
        ret = -1;
        goto exit;
    }

    /*
     * BLS layout version
     */
    if (snprintf(path, PATH_MAX, "%s/%s", conf->kernel_conf_bls_dir, file) < 0) {
        ret = -1;
        goto exit;
    }

    *fptr_dst = fopen(path, "w");
    if (fptr_dst == nullptr) {
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}

/*
 * Returns true of the row is modifed.
 * Changed layout = ... ==> layout = bls
 */
static bool adjust_row(size_t row_len, char *row_in, char *row_out) {
    const char *key = "layout";
    size_t key_len = strlen(key);
    char tmp[ROW_SZ] = {'\0'};
    char *tmp_ptr = nullptr;
    bool changed = false;

    row_out[0] = '\0';
    if (row_in == nullptr || row_in[0] == '\0') {
        return changed;
    }

    strlcpy(tmp, row_in, ROW_SZ);
    tmp_ptr = trim_string(tmp, ROW_SZ-1);
    if (tmp_ptr[0] == '\0') {
        return changed;
    }

    if (strncmp(tmp_ptr, key, key_len) == 0) {
        changed = true;
        (void)snprintf(row_out, row_len, "%s = bls\n", key);
    }
    return changed;
}

static int make_new_install_conf(SdBoot *conf) {
    int ret = 0;
    FILE *fptr_src = nullptr;
    FILE *fptr_dst = nullptr;

    ret = open_files(conf, &fptr_src, &fptr_dst);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Make the new one.
     */
    char row[ROW_SZ] = {'\0'};
    char row_mod[ROW_SZ] = {'\0'};
    char *ptr = nullptr;

    while (fgets(row, sizeof(row), fptr_src) != nullptr) {
        if (adjust_row(ROW_SZ, row, row_mod)) {
            ptr = row_mod;
        } else {
            ptr = row;
        }
        size_t bytes = strnlen(ptr, ROW_SZ);
        if (fwrite(ptr, sizeof(char), bytes, fptr_dst) < bytes) {
            ret = -1;
            goto exit;
        }
    }
        

exit:
    if (fptr_src != nullptr) {
        if (fclose(fptr_src) != 0) {
            perror(nullptr);
            ret = -1;
        }
    }
    if (fptr_dst != nullptr) {
        if (fclose(fptr_dst) != 0) {
            perror(nullptr);
            ret = -1;
        }
    }
    return ret;
}

int ki_make_kernel_conf_bls(SdBoot *conf) {
    int ret = 0;
    Array_str skip = {};

    ret = array_str_new(1, &skip);
    if (ret != 0) {
        goto exit;
    }
    skip.rows[0] = strdup("install.conf");

    /*
     * Make new kernel conf dir
     */
    ret = dir_dup_links(conf->kernel_conf_dir, conf->kernel_conf_bls_dir, &skip);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Make install.conf with layout=bls
     */
    ret = make_new_install_conf(conf);
    if (ret != 0) {
        goto exit;
    }

exit:
    array_str_free(&skip);
    return ret;
}
