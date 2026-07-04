// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * read key = value pairs from a file
 * Comments - lines starting with '#' are ignored
 *
 * Return:
 *
 *  0 = success
 *  1 = file not available
 * -1 = error
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-keyval.h"

enum ReadInfo { CHUNK = 32 };

/**
 * Ensures the KvList has enough allocated memory for a new element.
 */
static int update_list_mem(KvList *list) {
    if (list->num_elems_used >= list->num_elems) {
        if (kvlist_alloc(list->num_elems + CHUNK, list) != 0) {
            return -1;
        }
    }
    return 0;
}

/**
 * Validates a string length against the max value permitted.
 * If max_str_len is 0, bounds tracking is bypassed.
 */
static bool is_length_valid(const char *str, size_t max_len) {

    if (max_len == 0) {
        return true;
    }
    return strnlen(str, max_len + 1) <= max_len;
}


/**
 * Set KvElema value:
 *  elem->str_val or elem->int_val
 *
 * If the parsed value is an integer then type is set to
 * KV_INT and val is set in int_val.
 *
 * Otherwise type is set to KV_STR and value in str_val.
 *
 * Quoting forces the value to be treated as a string, even if it
 * looks numeric. For example:
 *   xxx = 12    => parsed as an integer (int_val = 12)
 *   xxx = "12"  => parsed as a string (str_val = "12", quotes stripped)
 */
static int assign_value(KvElem *elem, const char *raw_val, size_t max_len, bool was_quoted) {

    if (!is_length_valid(raw_val, max_len)) {
        return -1;
    }

    if (*raw_val != '\0') {
        int int_val = 0;
        if (!was_quoted && string_to_int(raw_val, &int_val)) {
            elem->type = KV_INT;
            elem->int_val = int_val;
            return 0;
        }

        elem->type = KV_STR;
        char *ptr = strdup(raw_val);
        if (ptr) {
            elem->str_val = ptr;
        } else {
            return -1;
        }

    } else {

        elem->type = KV_STR;
        char *ptr = strdup("");
        if (ptr) {
            elem->str_val = ptr;
        } else {
            return -1;
        }
    }

    return 0;
}

/*
 * If raw_val is wrapped in a single matching pair of quotes
 * ("..." or '...'), strip them in place and report that it was quoted.
 * Otherwise leave raw_val untouched.
 */
static bool strip_quotes_inplace(char *raw_val) {
    size_t len = strlen(raw_val);

    if (len < 2) {
        return false;
    }

    char first = raw_val[0];
    char last = raw_val[len - 1];

    if ((first != '"' && first != '\'') || first != last) {
        return false;
    }

    memmove(raw_val, raw_val + 1, len - 2);
    raw_val[len - 2] = '\0';
    return true;
}

/**
 * Parses a valid key=value string and store into the element.
 * - value is optional
 * - line must have at least "key = "
 * - any line without "key =" is ignored.
 *
 * Return:
 *  0 = success
 *  1 = skip (ignore this line)
 * -1 = error
 */
static int parse_key_val(KvElem *elem, char *trimmed, size_t max_len) {

    char *equal = strchr(trimmed, '=');
    if (!equal) {
        return 1;
    }

    *equal = '\0';
    char *raw_key = trim_str_inplace(trimmed);
    char *raw_val = trim_str_inplace(equal + 1);

    /*
     * Chec foe Missing key - skip if none
     */
    if (*raw_key == '\0') {
        return 1;
    }

    if (!is_length_valid(raw_key, max_len)) {
        return -1;
    }

    char *key_ptr = strdup(raw_key);
    if (key_ptr) {
        elem->key = key_ptr;
    } else {
        return -1;
    }

    bool was_quoted = strip_quotes_inplace(raw_val);

    return assign_value(elem, raw_val, max_len, was_quoted);
}


/**
 * Read file (at path) containing key = value pairs.
 * Comments are ignored.
 * Return:
 *  -1 = error
 *   0 = success
 *   1 = file not available.
 */
int parse_keyval_file(const char *path, KvList *list) {

    if (!path || !list) {
        return -1;
    }

    FILE *stream = fopen(path, "r");
    if (!stream) {
        return 1;
    }

    /*
     * Caller limit on length of strings allowed
     * - max_str_len = 0 means there is no limit.
     * - make sure to retian list->max_str_len after free list.
     */
    size_t max_len = list->max_str_len;
    kvlist_free(list);
    list->max_str_len = max_len;

    char *line = nullptr;
    size_t linelen = 0;
    int status = 0;

    while (getline(&line, &linelen, stream) != -1) {
        char *trimmed = trim_str_inplace(line);

        if (*trimmed == '\0' || *trimmed == '#') {
            continue;
        }

        if (update_list_mem(list) != 0) {
            status = -1;
            break;
        }

        KvElem *elem = &list->elems[list->num_elems_used];

        int ret = parse_key_val(elem, trimmed, max_len);
        if (ret == 0) {
            list->num_elems_used++;
        } else {
            kvelem_free(elem);

            if (ret < 0) {
                status = -1;
                break;
            }
            continue;
        }
    }

    free(line);

    if (fclose(stream) != 0) {
        perror("fclose error");
        status = -1;
    }

    if (status == 0) {
        (void)kvlist_alloc(list->num_elems_used, list);
    } else {
        kvlist_free(list);
    }

    return status;
}

