// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Struct with list of pointers to strings.
 * Each string has it's own length which is tracked in row_len.
 * Provides num_rows_used as a convenience when caller allocated in chunks
 * to reduce malloc() calls.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-boot-msg.h"
#include "sd-boot-utils.h"

/*
 * initialize the list of pointers with "num_rows" rows.
 */
int array_str_new(size_t num_rows, Array_str *arr) {

    if (!arr) {
        msg(MSG_ERR, "sd-boot: array strings bad input\n");
        return 1;
    }

    if (arr->num_rows > 0) {
        array_str_free(arr);
    }

    if (num_rows == 0) {
        return 0;
    }

    arr->rows = (char **)calloc(num_rows, sizeof(char *));
    if (!arr->rows) {
        perror("calloc fail");
        return -1;
    }

    arr->row_len = (size_t *)calloc(num_rows, sizeof(size_t));
    if (!arr->row_len) {
        perror("calloc fail");
        free((void *)arr->rows);
        arr->rows = nullptr;
        msg(MSG_ERR, "sd-boot: memory allocation error for %zu\n", num_rows);
        return -1;
    }

    arr->num_rows = num_rows;
    return 0;
}

/*
 * num_rows reduced.
 */
static int array_str_fewer_rows(size_t num_rows, Array_str *arr) {

    if (!arr || num_rows >= arr->num_rows) {
        return -1;
    }

    /*
     * Free up any unused rows.
     */
    for (size_t i = num_rows; i < arr->num_rows; i++) {
        if (arr->rows[i]) {
            free(arr->rows[i]);
            arr->rows[i] = nullptr;
        }
    }
    
    /*
     *  special case "free" - dont use realloc / reallocarray() even though ok to do.
     */
    if (num_rows == 0) {
        if (arr->rows) {
            free((void *)arr->rows);
        }
        if (arr->row_len) {
            free((void *)arr->row_len);
        }
        arr->rows = nullptr;
        arr->row_len = nullptr;
        arr->num_rows = 0;
        arr->num_rows_used = 0;
        
        return 0;
    } 

    void *tmp_ptr = nullptr;

    tmp_ptr = reallocarray((void *)arr->rows, num_rows, sizeof(char *));
    if (!tmp_ptr){
        perror("realloc failed");
        return -1;
    }
    arr->rows = (char **)tmp_ptr; 

    tmp_ptr = reallocarray((void *)arr->row_len, num_rows, sizeof(size_t));
    if (!tmp_ptr){
        perror("realloc failed");
        return -1;
    }
    arr->row_len = (size_t *)tmp_ptr; 

    arr->num_rows = num_rows;
    if (arr->num_rows_used > num_rows) {
        arr->num_rows_used = num_rows;
    }

    return 0;
}

/*
 * num_rows bigger
 */
static int array_str_more_rows(size_t num_rows, Array_str *arr) {
    
    if (!arr || num_rows <= arr->num_rows) {
        return -1;
    }

    void *tmp_ptr = nullptr;

    tmp_ptr = reallocarray((void *)arr->rows, num_rows, sizeof(char *));
    if (!tmp_ptr){
        perror("realloc failed");
        return -1;
    }
    arr->rows = (char **)tmp_ptr;

    tmp_ptr = reallocarray((void *)arr->row_len, num_rows, sizeof(size_t));
    if (!tmp_ptr){
        perror("realloc failed");
        return -1;
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

    return 0;
}

/*
 * Resize number of rows.
 *
 * Resize existing - num_rows can be higher or lower
 * Can only be used after being instantiated with array_str_new()
 * NB New mem is initilized to 0
 *
 */
int array_str_resize(size_t num_rows, Array_str *arr) {

    if (!arr) {
        msg(MSG_ERR, "sd-boot: memory alloc bad pointers\n");
        return -1;
    }

    /*
     * fewer rows
     */
    if (num_rows < arr->num_rows) {
        return array_str_fewer_rows(num_rows, arr);
    }

    /*
     * more rows
     */
    if (num_rows > arr->num_rows) {
        return array_str_more_rows(num_rows, arr);
    }

    return 0;
}

/*
 * Free all memory
 */
void array_str_free(Array_str *arr) {

    if (!arr) {
        return;
    }
    (void)array_str_resize(0, arr);

    arr->num_rows_used = 0;
}

/*
 * Refresh row len
 */
void array_str_refresh_row_len(Array_str *arr) {

    if (!arr) {
        return;
    }

    for (size_t i = 0; i < arr->num_rows; i++) {
        if (arr->rows[i]) {
            arr->row_len[i] = strlen(arr->rows[i]);
        } else {
            arr->row_len[i] = 0;
        }
    }
}
