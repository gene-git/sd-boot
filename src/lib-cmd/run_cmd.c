// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Run executable - standard execve() argyments and
 * child output ignored.
 * Needs _GNU_SOURCE (see meson.build)
 */
#include "sd-boot-cmd.h"

#include <sched.h>
#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

/**
 * Run command in argv with env envp
 * If no envp, then chold gets empty env.
 * e.g. argv ~ {"rsync", "-a", "/path/to/src", "/path/to/dest", nullptr}
 * Uses posix_spawn.
 *
 * :param argv: Command/args to run - standard execve() format. Array of strings
 *              command is first, then args, end with null.
 * :param envp: Associated environment. Same format as argv
 * :param child_ret_p: The child exit code.
 */
int run_cmd(char **argv, char **envp, int *child_ret_p) {
    pid_t pid = 0;
    int ret = 0;
    int ret_spawn = 0;
    int child_status = 0;
    int child_ret = -1;
    posix_spawnattr_t spawn_attr = {};

    if (argv[0] == nullptr || argv[0][0] == '\0') {
        return -1;
    }

    /*
     * good practice to reset child signals.
     */
    ret = init_spawn_attr(&spawn_attr);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Instantiate the child process
     */
    ret_spawn = posix_spawn(&pid, argv[0], nullptr, &spawn_attr, &argv[0], envp);

    if (ret_spawn == 0) {
        pid_t wait_ret = 0;
        wait_ret = waitpid(pid, &child_status, 0);
        if (wait_ret == pid) {
            if (WIFEXITED(child_status) != 0) {
                child_ret = WEXITSTATUS(child_status);
            }
        } else {
            child_ret = -1;
            perror(nullptr);
        }
    } else {
        ret = -1;
        child_ret = -1;
        perror(nullptr);
    }

    if (child_ret_p != nullptr) {
        *child_ret_p = child_ret;
    }

exit:
    if (posix_spawnattr_destroy(&spawn_attr) != 0) {
        perror(nullptr);
    }
    return ret;
}
