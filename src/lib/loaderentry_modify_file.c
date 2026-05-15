// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * The final phase of modifying the loader entry happens here.
 *
 * Read the existing entry file and write the new one with
 * the requested change provided by LoaderEntry structure.
 *
 * @return 0 = success, -1 = error
 */
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sd-boot.h"

enum Constants {
    ROW_SZ = 1024,
};

static int open_temp_file(const char *dir, char *path, FILE **fptr_p) {
    //
    // Open a temp file to write in directory "dir"
    //
    // @dir = directory where temp file is to reside.
    // @path_tmp = path to temp file (should be of size PATH_MAX)
    // @fptr_p = address of file stream. Open stream in *fptr_p
    //
    // Return:
    //   0 = all good
    //  -1 = error
    //
    int ret = 0;

    *fptr_p = nullptr;

    if (snprintf(path, PATH_MAX, "%s/%s", dir, "./tmp_file.XXXXXX") < 0) {
        ret = -1;
        goto exit;
    }

    int tmp_fd = mkstemp(path);
    if (tmp_fd == -1) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    // Convert to FILE stream 
    *fptr_p = fdopen(tmp_fd, "w");
    if (*fptr_p == nullptr) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}

static int open_entry_file(const char *dir, const char *file, char *path, FILE **fptr_p) {
    //
    // Open entry file for read
    // 
    int ret = 0;

    if (snprintf(path, PATH_MAX, "%s/%s", dir, file) < 0) {
        ret = -1;
        goto exit;
    }

    *fptr_p = fopen(path, "r");
    if (*fptr_p == nullptr) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
exit:
    return ret;
}

static int entry_row_modify(LoaderEntry *entry, size_t row_len, char *row_in, char *row_out) {
    //
    // Process one line
    // 
    int ret = 0;
    char *tag = nullptr;
    size_t tag_len = 0;

    if (entry->title != nullptr && entry->title[0] != '\0') {
        // "title" => update
        tag = "title";
        tag_len = strlen(tag);
        if (strncmp(row_in, tag, tag_len) == 0) {
            if (snprintf(row_out, row_len, "%s      %s\n", tag, entry->title) < 0) {
                ret = -1;
            }
            goto exit;
        }
    }

    if (entry->is_efi_tool) {
        // "options" => drop
        tag = "options";
        tag_len = strlen(tag);
        if (strncmp(row_in, tag, tag_len) == 0) {
            row_out[0] = '\0';
            goto exit;
        }

        // "linux ... " => "efi ... "
        tag = "linux";
        tag_len = strlen(tag);
        if (strncmp(row_in, tag, tag_len) == 0) {
            if (snprintf(row_out, row_len, "%s%s\n", "efi  ", &row_in[tag_len]) < 0) {
                ret = -1;
            }
            goto exit;
        }
    }

    // no mods made 
    strncpy(row_out, row_in, row_len);

exit:
    return ret;
}

int loaderentry_modify_file(LoaderEntry *entry) {
    //
    // Loader entry path: <dir>/<file>
    //
    // Set the loader entry:
    // - ^title xxx => "title <title>"
    // - ^linux xxx => "efi xxx"
    // - ^options xxx => remove
    //
    int ret = 0;
    char path[PATH_MAX] = {'\0'};
    char path_tmp[PATH_MAX] = {'\0'};
    FILE *tmp_fptr = nullptr;
    FILE *fptr = nullptr;

    if (entry == nullptr || entry->loader_entry_dir == nullptr || entry->loader_entry_file == nullptr) {
        return -1;
    }

    // Set up a temp file to write to.
    ret = open_temp_file(entry->loader_entry_dir, path_tmp, &tmp_fptr);
    if (ret != 0) {
        goto exit;
    }

    // existing entry file
    ret = open_entry_file(entry->loader_entry_dir, entry->loader_entry_file, path, &fptr);
    if (ret != 0) {
        goto exit;
    }

    //
    // read / edit 
    //
    char row[ROW_SZ] = {'\0'};
    char row_mod[ROW_SZ] = {'\0'};

    while (fgets(row, sizeof(row), fptr) != nullptr) {

        ret = entry_row_modify(entry, ROW_SZ, row, row_mod);
        if (ret != 0) {
            ret = -1;
            goto exit;
        }

        if (row_mod[0] != '\0') {
            size_t bytes = strnlen(row_mod, ROW_SZ);
            if (fwrite(row_mod, sizeof(char), bytes, tmp_fptr) < bytes) {
                ret = -1;
                goto exit;
            }
        }
    }

    //
    // close
    //
    (void) fclose(fptr);
    (void) fclose(tmp_fptr);
    fptr = nullptr;
    tmp_fptr = nullptr;

    // All well then rename the file (atomic)
    if (ret == 0) {
        if (rename(path_tmp, path) != 0) {
            perror(nullptr);
            (void) unlink(path_tmp);
            path_tmp[0] = '\0';
            ret = -1;
        }
    }
exit:
    if (tmp_fptr != nullptr) {
        (void) fclose(tmp_fptr);
    }
    if (fptr != nullptr) {
        (void) fclose(fptr);
    }
    if (ret != 0 && path_tmp[0] != '\0') {
        (void) unlink(path_tmp);
    }

    return ret;
}


