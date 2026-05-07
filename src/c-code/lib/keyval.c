// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * read config file
 * Contents can be 
 * - comments (# ...)
 * - key = value 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sd-boot.h"

static int line_to_key_val(ssize_t num_read, char *line, char **key_p, char **val_p) {
    /* 
     * parse one line
     * Returns 
     * -1 = error
     *  0 = found a key
     *  1 = line does not contain "<key> = <val>"
     *  If key is found then *key and *val point to what was found.
     */

    /*
     * sanity check
     */
    if (num_read < 0 || line == nullptr || *line == '\0') {
        return 1;
    }
    size_t num = (size_t)num_read;

    /*
     * skip comments / empty lines
     */
    char *ptr = trim_string(line, num);
    if (ptr[0] == '#' || ptr[0] == '\n' || ptr[0] == '\0') {
        return 1;
    }

    char *save_ptr = nullptr;
    *key_p = strtok_r(line, "=", &save_ptr);
    *val_p = strtok_r(nullptr, "=", &save_ptr);

    /*
     * key = required (val can be empty)
     */
    if (*key_p == nullptr) {
        return 1;
    }
    *key_p = trim_string(*key_p, MAX_KEY_LEN-1);

    if (*val_p != nullptr) {
        *val_p = trim_string(*val_p, MAX_KEY_LEN-1);
    }
    return 0;
}

static int match_key_to_elem(char *key, size_t num_elems, KvElem *elem, size_t *idx_p) {
    /*
     * Return the index of the matching by key elem in idx_p
     * Returns:
     * -1 = error
     *  0 = match found, *idx_p is the elem index
     *  1 = no match found
     */
    if (key == nullptr) {
        return -1;
    }

    for (size_t idx = 0; idx < num_elems; idx++) {
        if (strncmp(key, elem[idx].key, MAX_KEY_LEN-1) == 0) {
            *idx_p = idx;
            return 0;
        }
    }
    return 0;
}

static int save_key_val(char *key, char *val, size_t num_elems, KvElem *elem) {
    /*
     * Parse one line
     * Returns:
     * -1 = error
     *  0 = found a match
     *  1 = no match
     */

    if (key == nullptr || val == nullptr) {
        return 1;
    }

    /*
     * check if key is requested
     */
    size_t idx = 0;
    int ret = match_key_to_elem(key, num_elems, elem, &idx);
    if (ret != 0 || idx > num_elems) {
        return 1;
    }

    /*
     * int and string values only
     */
    if (elem[idx].type == CONF_INT) {
        elem[idx].val.v_int = str_to_int(val, elem[idx].lo_val_int, elem[idx].hi_val_int);
    } else {
        strncpy(elem[idx].val.v_str, val, sizeof(elem[idx].val.v_str));
    }

    return 0;
}

static bool key_check(ssize_t num_read, char *line, size_t num_elems, KvElem *elem) {
    /*
     * If key matches required key then save val,.
     * Returns true if a match otherwise 0
     */
    char *key = nullptr;
    char *val = nullptr;

    /*
     * look for (any key) line ~ "<key> = <val>"
     */
    if (line_to_key_val(num_read, line, &key, &val) != 0){
        return false;
    }

    /*
     * check and save if key matches a requested key
     */
    if (save_key_val(key, val, num_elems, elem) == 0) {
        // matched
        return true;
    }
    return false;
}

int read_kv_elems(const char *path, size_t num_elems, KvElem *elem, size_t *num_found_p) {
    /*
     * Read a file with "num_elems" key = value pairs
     * - comments, empty lines igmored
     * Args:
     *  path = file with key/val pairs
     *  num_elems = num elems provided to read
     *  elems = array of kv elems to parse
     * 
     * Each elem must provide the key to match. 
     * For integer values, bounds must also be provided. 
     * The first instance of each key is read - once all keys are found 
     * the results are returned.
     *
     * Return:
     * -1 = error
     *  0 = success reading num_elems (even when num_elems == 0)
     *  1 = file does not exist
     */
    int ret = 0;
    char *line = nullptr;
    char *line_copy = nullptr;

    if (num_elems == 0 || path == nullptr) {
        return 0;
    }

    if (access(path, F_OK) != 0) {
        /*
         * no such file
         */
        return 1;
    }

    FILE *fptr = nullptr;
    fptr = fopen(path, "r");            // NOLINT(concurrency-mt-unsafe)
    if (fptr == nullptr ){
        perror("Error opening file");
        ret = -1;
        goto exit;
    }

    /* 
     * To minimize malloc() calls, initialize "line" with reasonable value
     */
    size_t line_len = ROW_MAX;
    line = (char *)calloc(line_len, sizeof(char));
    if (line == nullptr) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    line_copy = (char *)calloc(line_len, sizeof(char));
    if (line_copy == nullptr) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    ssize_t num_read = 0;
    size_t num_found = 0;

    while ((num_read = getline(&line, &line_len, fptr)) > 0) {
        strncpy(line_copy, line, line_len);
        if (key_check(num_read, line_copy, num_elems, elem)) {
            num_found++;
            if (num_found == num_elems) {
                break;
            }
        }
    }

    /*
     * check for error
     */
    if (num_read < 0 && !feof(fptr)) {
        ret = -1;
        perror(nullptr);
    } 

    *num_found_p = num_found;

exit:
    if (fptr != nullptr) {
        if (fclose(fptr) != 0) {
            perror(nullptr);
        }
    }
    if (line != nullptr) {
        free((void *) line);
    }
    if (line_copy != nullptr) {
        free((void *) line_copy);
    }
    return ret;
}

int alloc_kv_elems(size_t num, KvElem **elem_p) {
    /*
     * allocate array of num elems
     * - caller / owner must free the pointer.
     */
    *elem_p = (KvElem *)calloc(num, sizeof(KvElem));
    if (*elem_p == nullptr) {
        perror(nullptr);
        return -1;
    }
    return 0;
}
