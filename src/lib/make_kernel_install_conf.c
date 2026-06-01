// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Create a copy of /etc/kernel/install.conf in /var/lib/sd-boot/kernel_conf_bls/install.conf
 * which uses bls layout.
 *
 * Check if any existing file exists and is correct before attempting to write a new one.
 *
 * If all good, returns 0
 *
 */
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sd-boot.h"

enum Const {
    CHUNK = 4096,
    NUM_KEYS = 3,
};

static int write_new_file(size_t bls_size, const char *bls, const char *dst) {
    int ret = 0;
    int fdes = -1;

    int flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    fdes = open(dst, flags, mode);
    if (fdes < 0) {
        perror(nullptr);
        return -1;
    }

    ssize_t written = write(fdes, bls, bls_size);
    if (written < 0) {
        ret = -1;
        goto exit;
    }

exit:
    if (fdes > 0) {
        (void)close(fdes);
    }
    return ret;
}


/*
 * Map source file into mem
 */
static int map_file(const char *src, void **file_map_p, size_t *file_size) {
    int ret = 0;
    int fdes = -1;
    struct stat stat = {};

    *file_map_p = MAP_FAILED;
    *file_size = 0;

    /*
     * Map src into mem
     */
    fdes = open(src, O_RDONLY | O_CLOEXEC);
    if (fdes < 0) {
        perror(nullptr);
        return -1;
    }

    if (fstat(fdes, &stat) < 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    if (stat.st_size == 0) {
        msg(MSG_ERR, "- sd-boot warning: empty file: %s\n", src);
        goto exit;
    }

    *file_map_p = mmap(nullptr, (size_t)stat.st_size, PROT_READ, MAP_SHARED, fdes, 0);
    if (*file_map_p == MAP_FAILED) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    *file_size = (size_t)stat.st_size;

exit:
    if (fdes > 0) {
        (void)close(fdes);
    }
    return ret;
}


/*
 * Deal with adjusting the layout line
 * line_start / line_end are unmodified.
 */
struct BlsInfo {
    char *line_start;
    char *line_end;

    char *this_line;
    size_t this_length;
    size_t num_used;
    size_t num_allocated;
    char *bls;
};


static int modify_one_line(KInstallMods *mods, struct BlsInfo *info) {
    int ret = 0;
    //const char *key = "layout";
    //size_t key_len = strlen(key);
    //const char *bls_line = "layout = bls\n";
    //const size_t bls_line_len = strlen(bls_line);
    void *ptr = nullptr;

    char *keys[NUM_KEYS] = {};
    char *changes[NUM_KEYS] = {};
    char *key = nullptr;
    char *change = nullptr;
    size_t key_len = 0;
    size_t change_len = 0;

    keys[0] = "layout";
    keys[1] = "initrd_generator";
    keys[2] = "uki_generator";

    changes[0] = mods->layout;
    changes[1] = mods->initrd_generator;
    changes[2] = mods->uki_generator;

    size_t line_len = info->line_end - info->line_start;

    /*
     * Line can only match 1 key or no keys
     */
    bool matched = false;
    for (size_t i = 0; i < NUM_KEYS; i++) {

        key = keys[i];
        key_len = strlen(key);
        change = changes[i];
        change_len = strlen(change);

        if (line_len >= key_len && strncmp(info->line_start, key, key_len) == 0) {
            matched = true;
            break;
        }
    }
    if (matched) {
        info->num_used += change_len;
        info->this_line = change;
        info->this_length = change_len;
    /* 
    if (line_len >= key_len && strncmp(info->line_start, key, key_len) == 0) {
        info->num_used += bls_line_len;
        info->this_line = (char *)bls_line;
        info->this_length = bls_line_len;
     */
    } else {
        info->num_used += line_len;
        info->this_line = info->line_start;
        info->this_length = line_len;
    }

    /*
     * Add space for newline
     */
    info->num_used++;
    if (info->num_used >= info->num_allocated) {
        info->num_allocated += info->num_used + CHUNK;
        ptr = realloc(info->bls, info->num_allocated * sizeof(char));
        if (ptr == nullptr) {
            msg(MSG_ERR, "! sd-boot mem allocation error\n");
            ret = -1;
            goto exit;
        }
        info->bls = (char *)ptr;
    }
exit:
    return ret;
}

/*
 * Map source file into memory, 
 * read into *new_install_conf_p layout to bls
 * return size (including null terminator).
 */
static int make_new_install(KInstallMods *mods, const void *file_map, size_t file_size, 
        char **bls_p, size_t *bls_size_p) {
    int ret = 0;
    struct BlsInfo info = {};

    *bls_p = nullptr;
    *bls_size_p = 0;

    const char *file_start = (const char *)file_map;
    const char *file_end = file_start + file_size;

    info.line_start = (char *)file_start;

    /*
     * buffer for new data
     */
    info.num_allocated = CHUNK;
    info.bls = (char *)malloc(CHUNK * sizeof(char));
    if (info.bls == nullptr) {
        msg(MSG_ERR, "! sd-boot mem allocation error\n");
        ret = -1;
        goto exit;
    }

    /*
     * read the mapped file
     */
    char *curr_position = nullptr;
    size_t total_length = 0;
    void *ptr = nullptr;

    while (info.line_start < file_end) {
        info.line_end = memchr(info.line_start, '\n', file_end - info.line_start);

        /*
         * Handle end of file without a newline
         */
        if (info.line_end == nullptr) {
            info.line_end = (char *)file_end;
        }

        /*
         * Adjust line starting with "layout" key
         */
        size_t num_used_so_far = info.num_used;

        ret = modify_one_line(mods, &info);
        if (ret != 0) {
            goto exit;
        }

        /*
         * this_length is without the newline
         */
        curr_position = info.bls + num_used_so_far;
        info.this_length++;
        memcpy((void *)curr_position, (const void *)info.this_line, info.this_length);
        total_length += info.this_length;

        /*
         * advance to end of line (skip over any trailing newline)
         */
        info.line_start = info.line_end;
        if (info.line_start < file_end && info.line_start[0] == '\n') {
            info.line_start++;
        }
    }

    /*
     * Keep only what's needed and null terminate
     */
    total_length += 1;
    if (total_length != info.num_allocated) {
        ptr = realloc(info.bls, total_length * sizeof(char));
        if (ptr == nullptr) {
            msg(MSG_ERR, "! sd-boot mem allocation error\n");
            ret = -1;
            goto exit;
        }
        info.bls = (char *)ptr;
    }
    info.bls[total_length - 1] = '\0';

    *bls_p = info.bls;
    *bls_size_p = total_length;

exit:
    if (ret != 0){
        if (info.bls != nullptr) {
            free((void *)info.bls);
        }
        *bls_p = nullptr;
        *bls_size_p = 0;
    }
    return ret;
}

int make_kernel_install_conf(KInstallMods *mods, const char *src, const char *dst) {
    int ret = 0;
    void *file_map = MAP_FAILED;
    size_t file_size = 0;
    char *bls = nullptr;
    size_t bls_size = 0;

    if (src == nullptr || dst == nullptr) {
        return -1;
    }

    /*
     * Map src into mem
     */
    ret = map_file(src, &file_map, &file_size);
    if (ret != 0) {
        return -1;
    }

    /*
     * Read src and change layout to bls
     */
    ret = make_new_install(mods, file_map, file_size, &bls, &bls_size);
    if (ret != 0) {
        goto exit;
    }

    if (munmap(file_map, file_size) == -1) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    /*
     * Write new file if not same.
     */
    if (file_size == 0) {
        ret = -1;
        goto exit;
    }

    bool changed = false;
    if (file_size != bls_size || bls_size == 0 || bls == nullptr) {
        changed = true;
    } else {
        if (memcmp(file_map, bls, file_size) != 0) {
            changed = true;
        }
    }
    
    if (changed) {
        ret = write_new_file(bls_size, (const char *)bls, dst);
        if (ret != 0) {
            goto exit;
        }
    }

exit:
    if (bls != nullptr) {
        free((void *)bls);
    }
    return ret;
}

