// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>

/**
 * Envp / Argc - array of strings with last element a nullptr;
 * Env variables and main(argc, argv).
 *
 * Count the number of elements up to but not including the nullptr.
 */
enum Constants {
    MAX_ARGS = 64,
};

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

    while (args[num_args] && num_args < max_args) {
        num_args++;
    }
    return num_args;
}

