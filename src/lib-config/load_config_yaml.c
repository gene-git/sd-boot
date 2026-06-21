// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read and parse sd-boot config.yaml file
 *
 * node is the root node of the document.
 * yaml parser note:
 * scalar_node_1: value_scalar_node_1
 * scalar_node_2: value_scalar_node_2
 *
 * mapping_node:
 *  key_scalar_1: key_1_value
 *  key_scalar_2: key_2_value
 *
 * sequence_node:
 *      seq_1: seq_1_val
 *      seq_2: seq_2_val
 */
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <yaml.h>

#include "sd-boot-utils.h"
#include "sd-boot-config.h"

enum Constants { VERB_MAX = 2 };

static int save_one(yaml_document_t *document, char *key_value, yaml_node_t *val_node, SdBoot *conf) {
    int ret = 0;
    char *value = nullptr;
    yaml_node_t *item_node = nullptr;
    yaml_node_item_t *item = nullptr;

    switch (val_node->type) {
        case YAML_SCALAR_NODE:
            if (strcmp(key_value, "verb") == 0) {
                value = (char *)val_node->data.scalar.value;
                conf->verb = str_to_int(value, 0, VERB_MAX);
            }
            break;

        case YAML_SEQUENCE_NODE:
            if (strcmp(key_value, "skip_kernel_plugins") == 0) {
                for (item = val_node->data.sequence.items.start;
                        item < val_node->data.sequence.items.top; item++) {
                    item_node = yaml_document_get_node(document, *item);

                    if (item_node->type != YAML_SCALAR_NODE) {
                        break;
                    }

                    value = (char *)item_node->data.scalar.value;
                    ret = array_str_add_string((const char *)value, &conf->skip_kernel_plugins);
                    if (ret != 0) {
                        goto exit;
                    }
                }
            }
            break;

        default:
            break;
    }

exit:
    return ret;
}

static int parse_config(yaml_document_t *document, yaml_node_t *node, SdBoot *conf) {
    int ret = 0;

    /*
     * sanity check
     */
    if (node->type != YAML_MAPPING_NODE) {
        return 0;
    }
    yaml_node_pair_t *pair = nullptr;

    /*
     * Walk the nodes
     */
    for (pair = node->data.mapping.pairs.start; pair < node->data.mapping.pairs.top; pair++) {
        
        yaml_node_t *key_node = yaml_document_get_node(document, pair->key);
        if (key_node->type != YAML_SCALAR_NODE) {
            continue;
        }

        /*
         * scalar keys for current node
         */
        char *key_value = (char *)key_node->data.scalar.value;

        yaml_node_t *val_node = yaml_document_get_node(document, pair->value);

        ret = save_one(document, key_value, val_node, conf); 
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    return ret;
}

/*
 * Return:
 * -1 = error
 *  0 = all good
 *  1 = config file not available or empty.
 */
int load_config_yaml(SdBoot *conf) {
    int ret = 0;
    FILE *fptr = nullptr;
    char path[PATH_MAX] = {};

    /*
     * file to read
     */
    ret = yaml_config_path(conf, path);
    if (ret != 0) {
        goto exit;
    }

    yaml_parser_t parser = {};
    yaml_document_t document = {};

    fptr = fopen(path, "r");
    if (!fptr) {
        ret = 1;
        goto exit;
    }

    /*
     * Initialize
     */
    if (!yaml_parser_initialize(&parser)) {
        ret = -1;
        goto exit;
    }
    yaml_parser_set_input_file(&parser, fptr);

    if (!yaml_parser_load(&parser, &document)) {
        ret = -1;
        goto exit;
    }

    yaml_node_t *root_node = yaml_document_get_root_node(&document);
    if (!root_node) {
        ret = 1;
        goto exit;
    }

    /*
     * Parse the config
     */
    ret = parse_config(&document, root_node, conf);
    if (ret != 0) {
        goto exit;
    }

exit:
    if (fptr) {
        (void)fclose(fptr);
    }
    yaml_document_delete(&document);
    yaml_parser_delete(&parser);

    return ret;
} 

