// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Struct with list of pointers to strings.
 * Each string has it's own length which is tracked in row_len.
 * Provides num_rows_used as a convenience when caller allocated in chunks
 * to reduce malloc() calls.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot.h"

/*
 * initialize the list of pointers with "num_rows" rows.
 */
int array_str_new(size_t num_rows, Array_str *arr) {
    /*
     * Initialize arr->rows, arr->row_len only.
     */
    int ret = 0;

    if (arr == nullptr) {
        msg(MSG_ERR, "sd-boot: array strings bad input\n");
        ret = 1;
        goto exit;
    }

    arr->num_rows = 0;
    arr->rows = (char **)calloc(num_rows, sizeof(char *));
    if (arr->rows == nullptr) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    arr->num_rows = num_rows;

    arr->row_len = (size_t *)calloc(num_rows, sizeof(size_t));
    if (arr->row_len == nullptr) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

exit:
    if (ret != 0 && arr != nullptr && arr->rows != nullptr) {
        msg(MSG_ERR, "sd-boot: memory allocation error for %zu\n", num_rows);
        array_str_free(arr);
    }
    return ret;
}

static int array_str_fewer_rows(size_t num_rows, Array_str *arr) {
    /*
     * num_rows reduced.
     */
    int ret = 0;

    /*
     * Free up mem for now unused rows.
     */
    for (size_t i = num_rows; i < arr->num_rows; i++) {
        if (arr->rows[i] != nullptr) {
            free((void *)arr->rows[i]);
        }
    }

    // special case "free" so we dont rely on realloc to free
    if (num_rows == 0) {
        if (arr->rows != nullptr) {
            free((void *)arr->rows);
        }
        if (arr->row_len != nullptr) {
            free((void *)arr->row_len);
        }
        arr->rows = nullptr;
        arr->row_len = nullptr;
        arr->num_rows = 0;

        goto exit;
    } 

    if (num_rows < arr->num_rows) {
        void *tmp_ptr = nullptr;

        tmp_ptr = reallocarray((void *)arr->rows, num_rows, sizeof(char *));
        if (tmp_ptr == nullptr){
            perror(nullptr);
            ret = -1;
            goto exit;
        }
        arr->rows = (char **)tmp_ptr; 

        tmp_ptr = reallocarray((void *)arr->row_len, num_rows, sizeof(size_t));
        if (tmp_ptr == nullptr){
            perror(nullptr);
            ret = -1;
            goto exit;
        }
        arr->row_len = (size_t *)tmp_ptr; 
    }
    arr->num_rows = num_rows;

exit:
    return ret;
}

static int array_str_more_rows(size_t num_rows, Array_str *arr) {
    //
    // num_rows bigger
    //
    int ret = 0;
    void *tmp_ptr = nullptr;

    tmp_ptr = reallocarray((void *)arr->rows, num_rows, sizeof(char *));
    if (tmp_ptr == nullptr){
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    arr->rows = (char **)tmp_ptr;

    tmp_ptr = reallocarray((void *)arr->row_len, num_rows, sizeof(size_t));
    if (tmp_ptr == nullptr){
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    arr->row_len = (size_t *)tmp_ptr;

    /*
     * set new pointers to null
     */
    for (size_t i = arr->num_rows; i < num_rows; i++) {
        arr->rows[i] = nullptr;
        arr->row_len[i] = 0;
    }

    arr->num_rows = num_rows;

exit:
    return ret;
}

/*
 * Resize number of rows.
 *
 * Resize existing - num_rows can be higher or lower
 * Can only be used after being instantiated with array_str_new()
 * NB New mem is not initilized to 0
 *
 */
int array_str_resize(size_t num_rows, Array_str *arr) {
    /*
     * Resize the array
     */
    int ret = 0;

    if (arr == nullptr) {
        msg(MSG_ERR, "sd-boot: memory alloc bad pointers\n");
        ret = -1;
        goto exit;
    }

    /*
     * fewer rows
     */
    if (num_rows < arr->num_rows) {
        ret = array_str_fewer_rows(num_rows, arr);
        if (ret != 0) {
            goto exit;
        }
    }

    if (num_rows == 0) {
        goto exit;
    }

    /*
     * more rows
     */
    if (num_rows > arr->num_rows) {
        ret = array_str_more_rows(num_rows, arr);
        if (ret != 0) {
            goto exit;
        }
    }

    arr->num_rows = num_rows;

exit:
    return ret;
}

/*
 * Free all memory
 */
int array_str_free(Array_str *arr) {
    return array_str_resize(0, arr);
}
