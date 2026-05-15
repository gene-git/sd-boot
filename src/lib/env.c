// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Environment variables
 */

#include <stdio.h>
#include <stdlib.h>

enum Constants {
    MAX_ARGS = 64,
};


int combined_env(size_t num1, char **envp1, size_t num2, char **envp2, char ***envp_p) {
    /*
     * Create new envp by combining 2 input envps.
     *
     * Mem is allocated here and caller is responsible for freeing *envp_p.
     *      free(*envp_p)
     * NB: Only the array of pointers is allocated  - contents are shared
     * i.e. caller must free(*envp_p)
     *
     * Every envp1 must have NULL as it's last element per execve requirement.
     *
     * num1 / num2 are number of non-null items in envp1 / envp2
     * Result has num1 + num2 elements plus the last nullptr.
     */
    int ret = 0;

    // safety
    if (envp_p == nullptr) {
        ret = -1;
        goto exit;
    }

    if (envp1 == nullptr) {
        num1 = 0;
    }
    if (envp2 == nullptr) {
        num2 = 0;
    }

    size_t num = num1 + num2 + 1;

    // trailing nullptr
    *envp_p = (char **)calloc(num, sizeof(char *));
    if (*envp_p == nullptr) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }

    // copy envp1
    for (size_t i = 0; i < num1; i++) {
        (*envp_p)[i] = envp1[i];
    }

    // copy envp2
    for (size_t i = 0; i < num2; i++) {
        (*envp_p)[num1 + i] = envp2[i];
    }

    // trailing null
    (*envp_p)[num - 1] = nullptr;

exit:
    return ret;
}

/*
 * Count number of elements when last element is a nullptr
 *
 * This is the format used by envp and argv.
 * Returns number of non-null elements.
 *
 * MAX_ARGS ensures loop always ends.
 */
int count_envp_argv(char *const args[]) {
    int num_args = 0;
    int max_args = MAX_ARGS;

    while (args[num_args] != nullptr && num_args < max_args) {
        num_args++;
    }
    return num_args;

}

