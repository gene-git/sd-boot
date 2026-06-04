// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Convert config file from toml to yaml
 *
 * Read /etc/sd-boot/config and write /etc/sd-boot/config.yaml
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "sd-boot.h"

enum Constants { CHUNK = 4096 };

/*
 * Keep the comment header from toml file
 */
static char *read_header(const char *file) {
    int ret = 0;
    FILE *fptr = nullptr;
    if (!file) {
        return nullptr;
    }
    char *title = "# /etc/sd-boot/config";
    char *title_new = "# /etc/sd-boot/config.yaml\n";

    fptr = fopen(file, "r");
    if (!fptr) {
        return nullptr;
    }

    char *hdr = nullptr;
    char *next = hdr;
    size_t current = 0;
    void *tmp = nullptr;
    size_t num_alloc = 0;
    size_t num_total = 0;

    char *buf = 0;
    size_t num_read = 0;

    while ((num_read = getline(&buf, &num_alloc, fptr)) > 0) {
        if (buf[0] != '#') {
            break;
        }
        if (strncmp(buf, title, strlen(title)) == 0) {
            free((void *)buf);
            buf = strdup(title_new);
            num_read = strlen(title_new);
        }
        num_total += num_read + 1;
        if (hdr) {
            tmp = realloc(hdr, num_total * sizeof(char));
            if (!tmp) {
                msg(MSG_ERR, "  ! sd-boot convert config memory allocation error\n");
                ret = -1;
                goto exit;
            }
            hdr = (char *)tmp;

        } else {
            hdr = calloc(num_total, sizeof(char));
            if (!hdr) {
                msg(MSG_ERR, "  ! sd-boot convert config memory allocation error\n");
                ret = -1;
                goto exit;
            }
        }
        next = hdr + current;
        if (strlcpy(next, buf, num_read + 1) != num_read) {
            ret = -1;
            goto exit;
        }
        current += num_read;
        free((void *)buf);
        buf = nullptr;
        num_alloc = 0;
    }

exit:
    if (!buf) {
        free((void *)buf);
    }
    if (ret != 0 && !hdr) {
        free((void *)hdr);
        hdr = nullptr;
    }
    return hdr;
}

void convert_config(SdBoot *conf) {
    int ret = 0;
    char *header = nullptr;
    char path[PATH_MAX] = {};

    msg(MSG_NORMAL, "⦁ sd-boot: converting config to yaml\n");
    /*
     * Read comment header from toml config
     */
    ret = toml_config_path(conf, path);
    if (ret != 0) {
        goto exit;
    }

    header = read_header(path);

    /*
     * Save yaml file
     */
    ret = yaml_config_path(conf, path);
    if (ret != 0) {
        goto exit;
    }

    (void) save_yaml_config(conf, (const char *)header, path);

exit:
    if (!header) {
        free((void *)header);
    }
}

