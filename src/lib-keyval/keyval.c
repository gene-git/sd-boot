// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Container for list of  (key, value) pairs which 
 * are read in from a file
 */
#include <stdlib.h>

#include "sd-boot-keyval.h"


void kvelem_free(KvElem *elem) {
    if (!elem) {
        return;
    }
    if (elem->key) {
        free(elem->key);
    }
    if (elem->str_val) {
        free(elem->str_val);
    }
    elem->key = nullptr;
    elem->str_val = nullptr;
    elem->type = KV_NONE;
    elem->int_val = 0;
}

void kvlist_free(KvList *list) {

    if (!list) {
        return;
    }

    if (list->elems) {

        for (size_t i = 0; i < list->num_elems; i++) {
            KvElem *elem = &list->elems[i];

            kvelem_free(elem);
        }
        free(list->elems);
    }

    list->elems = nullptr;
    list->num_elems = 0;
    list->num_elems_used = 0;
}

int kvlist_alloc(size_t num_elems, KvList *list) {

    if (!list) {
        return -1;
    }

    if (num_elems == 0) {
        kvlist_free(list);
        return 0;
    }

    if (num_elems > list->num_elems) {
        /*
         * Growing the list
         */
        void *ptr = realloc(list->elems, num_elems * sizeof(KvElem));
        if (!ptr) {
            return -1;
        }
        list->elems = (KvElem *)ptr;

        for (size_t i = list->num_elems; i < num_elems; i++) {
            KvElem *elem = &list->elems[i];

            elem->type = KV_NONE;
            elem->key = nullptr;
            elem->str_val = nullptr;
            elem->int_val = 0;
        }
        list->num_elems = num_elems;
    }

    else if (num_elems < list->num_elems) {
        /*
         * Shrinking
         */
        for (size_t i = num_elems; i < list->num_elems; i++) {
            KvElem *elem = &list->elems[i];
            kvelem_free(elem);
        }

        void *ptr = realloc(list->elems, num_elems * sizeof(KvElem));
        if (!ptr) {
            list->num_elems = num_elems;
            return -1;
        }
        list->elems = (KvElem *)ptr;
        list->num_elems = num_elems;
    }

    return 0;
}


