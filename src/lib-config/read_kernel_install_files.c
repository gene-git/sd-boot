// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * read and parse kernel-install "install.conf" config files.
 * See man kernel-install and below for more detail on "install.conf" 
 * and companion drop in files and the priority order etc before usr and otherwise alaphanumeric.
 */
#include <dirent.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "sd-boot-config.h"
#include "sd-boot-keyval.h"


/*
 *  Precedence order array for base files and directory roots
 */
static const char *const CONF_PATHS[] = {
    "etc/kernel",
    "run/kernel",
    "usr/local/lib/kernel",
    "usr/lib/kernel"
};
enum { CONF_PATHS_COUNT = 4, MIN_CONF_LEN = 6};

/**
 * Extraction helper: Iterates over a populated KvList, isolates your 
 * 3 target properties, and duplicates their values into your config struct.
 */
static void extract_values(const KvList *list, SdBoot *conf) {

    InstallConf *iconf = &conf->install_conf;

    for (size_t i = 0; i < list->num_elems_used; i++) {

        KvElem *elem = &list->elems[i];

        if (elem->type != KV_STR || !elem->key) {
            continue;
        }

        if (strcmp(elem->key, "layout") == 0) {
            if (iconf->layout) {
                free(iconf->layout);
            }
            iconf->layout = elem->str_val;
            elem->str_val = nullptr;

        } else if (strcmp(elem->key, "initrd_generator") == 0) {
            if (iconf->initrd_generator) {
                free(iconf->initrd_generator);
            }
            iconf->initrd_generator = elem->str_val;
            elem->str_val = nullptr;

        } else if (strcmp(elem->key, "uki_generator") == 0) {
            if (iconf->uki_generator) {
                free(iconf->uki_generator);
            }
            iconf->uki_generator = elem->str_val;
            elem->str_val = nullptr;

        } else if (strcmp(elem->key, "BOOT_ROOT") == 0) {
            if (iconf->boot_root) {
                free(iconf->boot_root);
            }
            iconf->boot_root = elem->str_val;
            elem->str_val = nullptr;

        } else if (strcmp(elem->key, "MACHINE_ID") == 0) {
            if (iconf->machine_id) {
                free(iconf->machine_id);
            }
            iconf->machine_id = elem->str_val;
            elem->str_val = nullptr;
        }
    }
}

/**
 * Filter function for scandir() to select only files ending with ".conf"
 *  - Must be longer than ".conf" (6)
 */
static int conf_file_filter(const struct dirent *entry) {

    if (entry->d_type != DT_REG && entry->d_type != DT_LNK) {
        return 0;
    }

    size_t len = strlen(entry->d_name);
    if (len < MIN_CONF_LEN) {
        return 0; 
    }
    return strcmp(entry->d_name + len - (MIN_CONF_LEN - 1), ".conf") == 0;
}

/**
 * Loops through a specific conf.d directory path, sorts files alphanumerically,
 * and parses their values sequentially to allow layered overwrites.
 */
static void process_dropin_dir(const char *base_path, KvList *list, SdBoot *conf) {

    int ret = 0;
    char dpath[PATH_MAX];
    size_t size = sizeof(dpath);

    ret = snprintf(dpath, size, "%s%s/install.conf.d", conf->root, base_path);
    if (ret < 0 ||  ret >= (int)size) {
        return;
    }

    struct dirent **namelist = nullptr;
    /*
     *  alphasort provides files in alphanumeric sort order 
     *  e.g., 10-local.conf before 50-dist.conf
     */
    int num = scandir(dpath, &namelist, conf_file_filter, alphasort);
    if (num < 0) {
        return;
    }

    for (int i = 0; i < num; i++) {
        char file[PATH_MAX];
        size = sizeof(file);

        ret = snprintf(file, size, "%s/%s", dpath, namelist[i]->d_name);
        if (ret < (int)size) {
            if (parse_keyval_file(file, list) == 0) {
                extract_values(list, conf);
            }
        }
        free(namelist[i]);
    }
    free((void *)namelist);
}

int read_kernel_install_files(SdBoot *conf) {

    if (!conf) {
        return -1;
    }

    KvList list = { .max_str_len = PATH_MAX };

    /*
     *  Find and parse the (monolithic) primary file
     */
    int ret = 0;
    for (int i = 0; i < CONF_PATHS_COUNT; i++) {
        char file[PATH_MAX] = {};
        size_t size = sizeof(file);

        ret = snprintf(file, size, "%s%s/install.conf", conf->root, CONF_PATHS[i]);
        if (ret < 0 || ret >= (int)size) {
            continue;
        }

        struct stat fst = {};
        if (stat(file, &fst) == 0) {
            /*
             *  First Match Wins:
             */
            if (parse_keyval_file(file, &list) == 0) {
                extract_values(&list, conf);
            }
            break;
        }
    }

    /*
     * Read any drop-in (*.conf) over-rides in priority order ---
     * Instead of short-circuiting, we sweep ALL directories from lowest priority 
     * (/usr/lib) up to highest priority (/etc) to layer configurations correctly.
     */
    for (int i = CONF_PATHS_COUNT - 1; i >= 0; i--) {
        process_dropin_dir(CONF_PATHS[i], &list, conf);
    }

    // Free the internal workspace structures cleanly
    kvlist_free(&list);

    return 0;
}
