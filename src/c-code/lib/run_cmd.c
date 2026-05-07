// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Run rsync to copy from src to dst
 */
#include <sched.h>
#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int run_cmd(char **argv, char **envp, int *child_ret_p) {
    /*
     * Run command in argv
     * - cmd - full path to executable
     * - argv is null terminated list of strings first arg is "name" of command
     *   e.g.
     *   {"rsync", "-a", "/path/to/src", "/path/to/dest", nullptr}
     * - we pass down empty environment - do not pass current env.
     */
    pid_t pid = 0;
    int ret = 0;
    int ret_spawn = 0;
    int child_status = 0;
    int child_ret = -1;

    if (argv[0] == nullptr || argv[0][0] == '\0') {
        return -1;
    }

    ret_spawn = posix_spawn(&pid, argv[0], nullptr, nullptr, &argv[0], envp);
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

    return ret;
}
