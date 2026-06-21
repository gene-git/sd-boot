// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-utils.h
 */
#ifndef SD_BOOT_UTILS_H
#define SD_BOOT_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>

/*
 * Array of pointers to strings.
 * Each string has it's own length.
 * num_rows_used - convenience for caller when allocting
 * rows in chunks to reduce number of malloc calls.
 */
typedef struct {
    size_t num_rows;
    size_t num_rows_used;
    char **rows;
    size_t *row_len;

} Array_str;

/*
 * Dynamic string
 */
typedef struct {
    size_t num_used;
    size_t num_alloc;
    char *bytes;
} Dynamic_str;

/*
 * Key val
 */
enum KV_CONSTANTS {
    KV_MAX_KEY_LEN = 64,
    KV_MAX_VAL_LEN = 256,
};

/*
 * Config element
 */
typedef enum {
    CONF_INT = 1,
    CONF_STR = 2,
    CONF_FLOAT = 3,
} KvElemType;

typedef struct {
    // each item in config file.
    // key / value (int or string)
    char *key;
    KvElemType type;
    int max_str_len;
    int lo_val_int;
    int hi_val_int;
    union {
        int v_int;
        char v_str[KV_MAX_VAL_LEN];
    } val;

} KvElem;


/*
 * Function declarations
 */
int array_str_new(size_t num_rows, Array_str *arr);
int array_str_resize(size_t num_rows, Array_str *arr);
int array_str_free(Array_str *arr);
int array_str_move(Array_str *arr_1, Array_str *arr_2);
int array_str_add_string(const char *string, Array_str *arr);
int array_str_dup(Array_str *arr_1, Array_str *arr_2);
int array_str_copy_rows(Array_str *arr_1, Array_str *arr_2);
int array_str_null_terminate(Array_str *arr);
void array_str_refresh_row_len(Array_str *arr);

int current_datetime_str(size_t buflen, char *buf);

int copy_file(const char *src, const char *dst);
int dir_dup_links(const char *src, const char *dst, Array_str *skips);
int dynamic_str_alloc(size_t num, Dynamic_str *str);

int read_kv_elems(const char *path, size_t num, KvElem *elem, size_t *num_found_p);
int alloc_kv_elems(size_t num, KvElem **elem_p);

int count_envp_argv(char *const args[]);
int file_list_glob(const char *pattern, Array_str *files);
char *get_one_line(char **state_p); 

int read_file(const char *path, Array_str *arr);
char *read_file_first_row(const char *path);
int read_one_line_fd(int fdes, char *buf, size_t len);
int makedir(const char *path, mode_t mode);
int makepath(const char *path, mode_t mode);
int make_symlink(const char *target, const char *linkname);
void strip_file_extension(char *filename, const char *ext);
int str_to_int(char *str, int low_value, int high_value);

int path_add_slash(char *path, char **path_p);
int remove_file(char *path);
int rm_rf(const char *path);

bool string_in_list(const char *name, size_t num_names, char **names);
char *trim_string(char *str, size_t max_len);
void strip_whitespace(char *str);

bool efivars_available();
bool unshare_available();

#endif
