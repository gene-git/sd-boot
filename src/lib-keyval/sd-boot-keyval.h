// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-utils.h
 */
#ifndef SD_BOOT_KEYVAL_H
#define SD_BOOT_KEYVAL_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    KV_NONE = 0,
    KV_INT = 1,
    KV_STR = 2,
} KvType;

typedef struct {
    KvType type;
    char *key;
    int int_val;
    char *str_val;
} KvElem;

typedef struct {
    KvElem *elems;
    size_t num_elems;
    size_t num_elems_used;
    size_t max_str_len;
} KvList;


/*
 * Function decls
 */
bool string_to_long(const char *str, long *long_val);
bool string_to_int(const char *str, int *int_val);
char *trim_str_inplace(char *str);

void kvelem_free(KvElem *elem);
void kvlist_free(KvList *list);
int kvlist_alloc(size_t num_elems, KvList *list);
int parse_keyval_file(const char *path, KvList *list);

#endif
