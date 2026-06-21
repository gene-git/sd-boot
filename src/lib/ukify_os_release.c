// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Make a copy of /usr/lib/os-release and change
 * NAME= (same as PRETTY_NAME)
 * PRETTY_NAME=(-> new-name - such as a kernel package name)
 * BUILD_ID=(-> new build id such as kernel package version.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"
#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

enum Constants {
    MEM_CHUNK = 1024,
};


/*
 * Adjust the value 
 */
static const char *adjust_value(const char *name, const char *build_id, const char *key, const char *value) {
    const char *key_name = "NAME";
    const char *key_pretty_name = "PRETTY_NAME";
    const char *key_build_id = "BUILD_ID";

    if (!key) {
        return value;
    }

    if (strncmp(key, key_name, strlen(key_name)) == 0) {
        return name;
    }

    if (strncmp(key, key_pretty_name, strlen(key_pretty_name)) == 0) {
        return name;
    }

    if (strncmp(key, key_build_id, strlen(key_build_id)) == 0) {
        return build_id;
    }
    return value;
}

static char *line_adjusted(const char *name, const char *build_id, char *line, size_t *total_len) {
    int ret = 0;
    char *equals = nullptr;
    size_t key_len = 0;
    char *value = nullptr;
    const char *value_new = nullptr;

    if (!line || line[0] == '\0') {
        return nullptr;
    }

    /*
     * missing key?
     */
    equals = strchr(line, '=');
    if (!equals || equals == line) {
        return nullptr;
    }

    key_len = equals - line;
    if (key_len == 0) {
        return nullptr;
    }

    value = equals + 1;
    if (value[0] == '\n' || value[0] == '\0') {
        return nullptr;
    }

    value_new = adjust_value(name, build_id, line, value);
    if (!value_new) {
        return nullptr;
    }

    /*
     * Room for '=', '\n' and trailing null
     */
    size_t value_len = strlen(value_new); 
    size_t num_alloc = key_len + value_len + 3;

    Dynamic_str str = {};
    ret = dynamic_str_alloc(num_alloc, &str);
    if (ret != 0) {
        msg(MSG_ERR, "  sd-boot: ukify_os_release: mem allocation error\n");
        return nullptr;
    }

    int num_bytes = 0;
    num_bytes = snprintf(str.bytes, num_alloc, "%.*s=%s\n", (int)key_len, line, value_new);
    if ((size_t)num_bytes >= num_alloc) {
        msg(MSG_ERR, "  sd-boot: ukify_os_release: unexpected write error\n");
        free((void *)str.bytes);
        return nullptr;
    }

    *total_len = num_alloc;

    return str.bytes;
}


static int append_line(char *line_new, size_t len_new, Dynamic_str *str) {
    int ret = 0;
    /*
     * Check have enough space.
     */
    if (str->num_used + len_new >= str->num_alloc) {
        ret = dynamic_str_alloc(str->num_used + MEM_CHUNK, str);
        if (ret != 0) {
            goto exit;
        }
    }

    /*
     * append the new line starting at the null.
     */

    char *ptr = nullptr;
    if (str->num_used == 0) {
        memcpy(str->bytes, line_new, len_new);
        str->num_used += len_new;
    } else {
        ptr = str->bytes + str->num_used - 1;
        memcpy(ptr, line_new, len_new);
        str->num_used += len_new - 1;
    }

exit:
    return ret;
}


/**
 * Generate os-release as string.
 *
 * Created as a copy of /usr/lib/os-release with modifications:
 * - NAME= (same as PRETTY_NAME)
 * - PRETTY_NAME=(-> new-name - such as a kernel package name)
 * - BUILD_ID=(-> new build id such as kernel package version.
 *
 * Caller responsible for freeing the memery.
 */
int ukify_os_release(const char *name, const char *build_id, Dynamic_str *os_release) {

    int ret = 0;
    FILE *fin = nullptr;
    char *line = nullptr;
    char *line_new = nullptr;
    const char *src = "/usr/lib/os-release";

    fin = fopen(src, "r");
    if (!fin ) {;
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    ssize_t num_read = 0;
    size_t line_len = 0;
    size_t len = 0;
    size_t len_adjusted = 0;

    ret = dynamic_str_alloc(MEM_CHUNK, os_release);
    if (ret != 0) {
        goto exit;
    }

    while ((num_read = getline(&line, &line_len, fin) != -1)) {
        /*
         * Read and write one line at a time
         */
        if (num_read == 0 || line[0] == '\0' || line[0] == '#') {
            continue;
        }

        /*
         * drop newline
         * - num_read is not num_chars read as documented but is 1.
         *   so cannot be used to remove newline
         */
        len = strnlen(line, line_len);
        if (line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        /*
         * len_adjusted includes trailing null
         */
        line_new = line_adjusted(name, build_id, line, &len_adjusted);
        if (!line_new ) {
            continue;
        }

        ret = append_line(line_new, len_adjusted, os_release);
        if (ret != 0) {
            ret = -1;
            goto exit;
        }

        free((void *)line_new);
        line_new = nullptr;
    }

    ret = dynamic_str_alloc(os_release->num_used, os_release);
    if (ret != 0) {
        goto exit;
    }

exit:
    if (line) {
        free((void *)line);
    }
    if (line_new) {
        free((void *)line_new);
    }
    if (fin && fclose(fin) != 0) {
        ret = -1;
        perror(nullptr);
    };

    return ret;
}

