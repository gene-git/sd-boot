// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Write SdBoot config file in yaml format.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

#include "sd-boot-msg.h"
#include "sd-boot-config.h"
#include "sd-boot-utils.h"

enum Const {
    CONF_DATE_LEN = 32,
    CONF_MAX_VAL_LEN = 256,
};


/*
 * Write header and date
 */
static int write_header(const char *hdr, FILE *fptr) {
    int ret = 0;
    size_t written = 0;
    size_t bytes = 0;

    /*
     * Caller provided header
     */
    if (hdr) {
        bytes = strlen(hdr);
        if (bytes > 0) {
            written = fwrite(hdr, sizeof(char), bytes, fptr);
            if (written < bytes) {
                msg(MSG_ERR, "Error writing header yaml config\n");
                ret = -1;
                goto exit;
            }
        }
    }
    /*
     * Comment and datetime
     */
    const char *note = 
        "# Note: this file replaces /etc/sd-boot/config\n"
        "#       Going forward, please make any changes to this file\n"
        "#\n";
    char datetime[CONF_DATE_LEN] = {};
    char comment[CONF_MAX_VAL_LEN] = {};
    if (current_datetime_str(CONF_DATE_LEN, datetime) != 0) {
        datetime[0] = '\0';
    }
    if (snprintf(comment, CONF_MAX_VAL_LEN, "%s# auto created from config %s\n#\n", note, datetime) > 0) {
        bytes = strlen(comment);
        written = fwrite(comment, sizeof(char), bytes, fptr);
        if (written < bytes) {
            msg(MSG_ERR, "Error writing yaml config  file\n");
            ret = -1;
            goto exit;
        }
    }


exit:
    return ret;
}

int save_yaml_config(SdBoot *conf, const char *hdr, const char *path) {
    int ret = 0;
    FILE *fptr = nullptr;
    yaml_emitter_t emitter = {};
    yaml_document_t document = {};
    int root_id = -1;
    int skip_id = -1;

    fptr = fopen(path, "w");
    if (!fptr) {
        ret = -1;
        goto exit;
    }

    /*
     * Write any header
     */
    ret = write_header(hdr, fptr);
    if (ret != 0) {
        goto exit;
    }

    /*
     * initialize yaml 
     */
    if (!yaml_emitter_initialize(&emitter)) {
        ret = -1;
        goto exit;
    }

    yaml_emitter_set_output_file(&emitter, fptr);
    yaml_emitter_set_indent(&emitter, 2);
    yaml_emitter_set_canonical(&emitter, 0);

    if (!yaml_document_initialize(&document, nullptr, nullptr, nullptr, 1, 1)) {
        ret = -1;
        goto exit;
    }

    /*
     * build the node tree
     */
    root_id = yaml_document_add_mapping(&document, nullptr, YAML_BLOCK_MAPPING_STYLE);

    yaml_char_t *yaml_str = nullptr;

    /*
     * verb
     */
    char val_str[CONF_MAX_VAL_LEN] = {};
    if (snprintf(val_str, CONF_MAX_VAL_LEN, "%d", conf->verb) < 0) {
        ret = -1;
        goto exit;
    }

    yaml_str = (yaml_char_t *)"verb";
    int verb_key = yaml_document_add_scalar(&document, nullptr, yaml_str, -1, YAML_PLAIN_SCALAR_STYLE); 

    yaml_str = (yaml_char_t *)val_str;
    int verb_val = yaml_document_add_scalar(&document, nullptr, yaml_str, -1, YAML_PLAIN_SCALAR_STYLE); 
    if (!yaml_document_append_mapping_pair(&document, root_id, verb_key, verb_val)) {
        ret = -1;
        goto exit;
    }

    /*
     * skip_kernel_plugins
     * - list of values.
     * - 
     */
    skip_id = yaml_document_add_sequence(&document, nullptr, YAML_ANY_SEQUENCE_STYLE);

    for (size_t i = 0; i < conf->skip_kernel_plugins.num_rows; i++) {
        yaml_str = (yaml_char_t *)conf->skip_kernel_plugins.rows[i];

        int item_id = yaml_document_add_scalar(&document, nullptr, yaml_str, -1, YAML_PLAIN_SCALAR_STYLE);
        if (!yaml_document_append_sequence_item(&document, skip_id, item_id)) {
            ret = -1;
            goto exit;
        }
    }

    yaml_str = (yaml_char_t *)"skip_kernel_plugins";
    int skip_plugins_key = yaml_document_add_scalar(&document, nullptr, yaml_str, -1, YAML_PLAIN_SCALAR_STYLE);
    yaml_document_append_mapping_pair(&document, root_id, skip_plugins_key, skip_id);


    /*
     * Save file
     */
    if (!yaml_emitter_dump(&emitter, &document)) {
        ret = -1;
        goto exit;
    }

exit:
    yaml_document_delete(&document);
    yaml_emitter_delete(&emitter);

    if (fptr) {
        (void)fclose(fptr);
    }
    return ret;
}
