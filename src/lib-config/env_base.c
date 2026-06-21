// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Iniitialize env 
 * - PATH 
 * In test we need additional env variables. e.g.
 * LD_LIBRARY_PATH - allows tests using shared lib in developement / build area.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "sd-boot-utils.h"

static int add_one_var(const char *name, const char *value, Array_str *env_base) {
    int ret = 0;
    char *env_var = nullptr;
    size_t size = 0;

    if (!name) {
        return 0;
    }

    /*
     * Mem to hold: name=value + null
     */
    size = strlen(name) + 2;
    if (value) {
        size += strlen(value);
    }

    env_var = (char *)calloc(size, sizeof(char));
    if (!env_var) {
        ret = -1;
        goto exit;
    }
    if (value) {
        ret = snprintf(env_var, size, "%s=%s", name, value);
    } else {
        ret = snprintf(env_var, size, "%s=", name);
    }

    if (ret < 0) {
        ret = -1;
        goto exit;
    }

    /*
     * Add env_var to environ
     */
    size_t ind_new = env_base->num_rows;
    ret = array_str_resize(env_base->num_rows + 1, env_base);
    if (ret < 0) {
        goto exit;
    }
    env_base->rows[ind_new] = env_var;
    env_var = nullptr;

exit:
    if (env_var) {
        free((void *)env_var);
    }
    return ret;
}

static int config_set_test_env(Array_str *env_base) {
    int ret = 0;
    char *value = nullptr;
    const char *names[] = {
        "LD_LIBRARY_PATH",
        "SDB_DEV_TEST",
        "MALLOC_PERTURB_",
        "UBSAN_OPTIONS",
        "ASAN_OPTIONS",
        "MSAN_OPTIONS",
        "MESON_TEST_ITERATION",
        "MSAN_OPTIONS",
    };

    size_t num_names = sizeof(names) / sizeof(names[0]);

    for (size_t i = 0; i < num_names; i++) {
        value = getenv(names[i]);               // NOLINT(concurrency-mt-unsafe)
        if (value) {
            ret = add_one_var(names[i], (const char *)value, env_base);
            if (ret != 0) {
                goto exit;
            }
        }
    }


exit:
    return ret;
}

int config_set_base_env(bool test, Array_str *env_base) {

    int ret = 0;
    const char *name = "PATH";
    const char *value = "/usr/bin";

    array_str_free(env_base);

    ret = add_one_var(name, value, env_base);
    if (ret != 0) {
        goto exit;
    }

    if (test) {
        ret = config_set_test_env(env_base);
        if (ret != 0) {
            goto exit;
        }
    }

    ret = array_str_null_terminate(env_base);
    if (ret != 0) {
        goto exit;
    }

    array_str_refresh_row_len(env_base);

exit:
    return ret;
}

