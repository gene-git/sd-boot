// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-utils.h
 */
#ifndef SD_BOOT_UTILS_H
#define SD_BOOT_UTILS_H

#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/stat.h>

typedef enum {
    MKDIR_MODE_DEF = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH,
    WRITE_FLAGS_DEF = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC,
    WRITE_MODE_DEF = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
} FileConstants;


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
 * Function declarations
 */
int array_str_new(size_t num_rows, Array_str *arr);
int array_str_resize(size_t num_rows, Array_str *arr);
void array_str_free(Array_str *arr);
int array_str_copy_move_prep(Array_str *arr_1, Array_str *arr_2, size_t *first_new_row);
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
void dynamic_str_free(Dynamic_str *str);

int count_envp_argv(char *const args[]);
int file_list_glob(const char *pattern, Array_str *files);
char *get_one_line(char **ptr_p);

int read_file(const char *path, Array_str *arr);
char *read_file_first_row(const char *path);
int makedir(const char *path, mode_t mode);
int makepath(const char *path, mode_t mode);
int make_symlink(const char *target, const char *linkname);
void strip_file_extension(char *filename, const char *ext);
int str_to_int(char *str, int low_value, int high_value);

int path_add_slash(char *path, char **path_p);
int remove_file(const char *path);
int rm_rf(const char *path);

bool string_in_list(const char *name, size_t num_names, char **names);
char *trim_string(char *str, size_t max_len);

bool efivars_available();
bool unshare_available();

int write_file(const char *data, size_t data_size, int flags, mode_t mode, const char *dest_path);

#endif
