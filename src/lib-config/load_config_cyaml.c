// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Read and parse sd-boot config.yaml file using libcyaml.
 *
 * Schema:
 *   verb: <integer>
 *   skip_kernel_plugins:
 *     - <string>
 *     - <string>
 *     ...
 */
#include <cyaml/cyaml.h>
#include <linux/limits.h>

#include "sd-boot-msg.h"
#include "sd-boot-config.h"
#include "sd-boot-utils.h"

/*
 * Plain struct libcyaml populates. libcyaml can't fill SdBoot's
 * Array_str directly, so we load into this, then transfer values
 * into conf below.
 */
typedef struct {
    int verb;
    char **skip_kernel_plugins;
    unsigned skip_kernel_plugins_count;
} YamlConfig;

/*
 * Schema for one entry in the skip_kernel_plugins sequence.
 */
static const cyaml_schema_value_t plugin_entry_schema = {
    CYAML_VALUE_STRING(CYAML_FLAG_POINTER, char, 0, CYAML_UNLIMITED),
};

/*
 * Schema for the top-level mapping (verb, skip_kernel_plugins).
 * Both fields are optional - an empty or partial file is fine.
 *
 * *Note*: We use libcyaml vs 1.x
 * For libcyaml version 2.0 (if it is ever released)
 *   - drop (unsigned int) casts on flag OR flag:
 *     CYAML_FLAG_OPTIONAL | CYAML_FLAG_POINTER,
 * Rest can remain same.
 */
static const cyaml_schema_field_t top_mapping_schema[] = {
    CYAML_FIELD_INT(
        "verb", CYAML_FLAG_OPTIONAL,
        YamlConfig, verb),
    CYAML_FIELD_SEQUENCE(
        "skip_kernel_plugins", (unsigned int)CYAML_FLAG_OPTIONAL | (unsigned int)CYAML_FLAG_POINTER,
        YamlConfig, skip_kernel_plugins,
        &plugin_entry_schema, 0, CYAML_UNLIMITED),
    CYAML_FIELD_END,
};

static const cyaml_schema_value_t top_schema = {
    CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER, YamlConfig, top_mapping_schema),
};

static const cyaml_config_t cyaml_cfg = {
    .log_fn = cyaml_log,
    .mem_fn = cyaml_mem,
    .log_level = CYAML_LOG_WARNING,
};

/*
 * Transfer values out of the libcyaml-populated struct and into conf.
 * Return: 0 on success, -1 on memory allocation error.
 */
static int save_config(const YamlConfig *ycfg, SdBoot *conf) {
    int ret = 0;

    conf->verb = ycfg->verb;
    set_verb_level(conf->verb);

    for (unsigned i = 0; i < ycfg->skip_kernel_plugins_count; i++) {
        ret = array_str_add_string(ycfg->skip_kernel_plugins[i], &conf->skip_kernel_plugins);
        if (ret != 0) {
            break;
        }
    }

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
    char path[PATH_MAX] = {};
    YamlConfig *ycfg = nullptr;
    cyaml_err_t err = CYAML_OK;

    /*
     * file to read
     */
    ret = yaml_config_path(conf, path);
    if (ret != 0) {
        return ret;
    }

    err = cyaml_load_file(path, &cyaml_cfg, &top_schema, (cyaml_data_t **)&ycfg, nullptr);

    if (err == CYAML_ERR_FILE_OPEN) {
        /*
         * config file not available - not an error.
         */
        return 1;
    }

    if (err != CYAML_OK) {
        msg(MSG_ERR,"!  sd-boot: yaml config parse error: %s\n", cyaml_strerror(err));
        return -1;
    }

    if (!ycfg) {
        /*
         * Empty document.
         */
        return 1;
    }

    ret = save_config(ycfg, conf);

    cyaml_free(&cyaml_cfg, &top_schema, ycfg, 0);

    return ret;
}
