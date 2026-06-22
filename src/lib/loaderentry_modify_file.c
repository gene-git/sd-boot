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
    /*
     * Open entry file for read
     * Return -1 if open fails.
     */ 
    int ret = 0;

    if (snprintf(path, PATH_MAX, "%s/%s", dir, file) < 0) {
        ret = -1;
        goto exit;
    }

    *fptr_p = fopen(path, "r");
    if (*fptr_p == nullptr) {
        ret = -1;
        goto exit;
    }
exit:
    return ret;
}

/*
 * Adjust title
 * Returns:
 *  -1 = error
 *   0 = all good
 *  mofified set true if row was modified.
 */
static int title_update(LoaderEntry *entry, size_t row_len, char *row_in, char *row_out, bool *modified) {
    const char *tag = "title";
    const size_t tag_len = strlen(tag);
    *modified = false;

    if (strncmp(row_in, tag, tag_len) == 0) {
        if (snprintf(row_out, row_len, "%s      %s\n", tag, entry->title) < 0) {
            return -1;
        }
        *modified = true;
        return 0;
    }
    return 0;
}

/*
 * Adjust efi tool entry:
 *  -1 = error
 *   0 = all good
 *  mofified set true if row was modified.
 */
static int efi_tool_update(size_t row_len, char *row_in, char *row_out, bool *modified) {
    char *tag = nullptr;
    size_t tag_len = 0;
    *modified = false;

    /*
     * "options" => drop
     */
    tag = "options";
    tag_len = strlen(tag);
    if (strncmp(row_in, tag, tag_len) == 0) {
        row_out[0] = '\0';
        *modified = true;
        return 0;
    }

    /*
     * "linux ... " => "efi ... "
     */
    tag = "linux";
    tag_len = strlen(tag);
    if (strncmp(row_in, tag, tag_len) == 0) {
        if (snprintf(row_out, row_len, "%s%s\n", "efi  ", &row_in[tag_len]) < 0) {
            return -1;
        }
        *modified = true;
        return 0;
    }

    /*
     * "uki ... " => "efi ... "
     */
    tag = "uki";
    tag_len = strlen(tag);
    if (strncmp(row_in, tag, tag_len) == 0) {
        if (snprintf(row_out, row_len, "%s%s\n", "efi  ", &row_in[tag_len]) < 0) {
            return -1;
        }
        *modified = true;
        return 0;
    }
    return 0;
}

static int entry_row_modify(LoaderEntry *entry, size_t row_len, char *row_in, char *row_out) {
    /*
     * Process one line
     */ 
    int ret = 0;
    bool modified = false;

    if (entry->title && entry->title[0] != '\0') {
        ret = title_update(entry, row_len, row_in, row_out, &modified);
        if (ret < 0 || modified) {
            goto exit;
        }
    }

    if (entry->is_efi_tool) {
        ret = efi_tool_update(row_len, row_in, row_out, &modified);
        if (ret < 0 || modified) {
            goto exit;
        }
    }

    /*
     * no mods made 
     */
    if (strlcpy(row_out, row_in, row_len) >= row_len) {
        ret = -1;
        goto exit;
    }

exit:
    return ret;
}

/*
 * Loader entry path: <dir>/<file>
 *
 * Set the loader entry:
 * - ^title xxx => "title <title>"
 * - ^linux xxx => "efi xxx"
 * - ^options xxx => remove
 */
int loaderentry_modify_file(LoaderEntry *entry) {
    int ret = 0;
    char path[PATH_MAX] = {};
    char path_tmp[PATH_MAX] = {};
    FILE *tmp_fptr = nullptr;
    FILE *fptr = nullptr;

    if (!entry || !entry->loader_entry_dir || !entry->loader_entry_file || 
            entry->loader_entry_file[0] == '\0') {
        return -1;
    }

    /* 
     * existing entry file
     * - load entry is only relevant for bls. So
     *   not an error if no loader entry file found.
     */
    ret = open_entry_file(entry->loader_entry_dir, entry->loader_entry_file, path, &fptr);
    if (ret != 0) {
        ret = 0;
        goto exit;
    }

    /*
     * Set up a temp file to write to.
     */
    ret = open_temp_file(entry->loader_entry_dir, path_tmp, &tmp_fptr);
    if (ret != 0) {
        goto exit;
    }

    /*
     * read / edit 
     */
    char row[ROW_SZ] = {};
    char row_mod[ROW_SZ] = {};

    while (fgets(row, sizeof(row), fptr)) {

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

    /*
     * close
     */
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
    if (tmp_fptr) {
        (void) fclose(tmp_fptr);
    }
    if (fptr) {
        (void) fclose(fptr);
    }
    if (ret != 0 && path_tmp[0] != '\0') {
        (void) unlink(path_tmp);
    }

    return ret;
}


