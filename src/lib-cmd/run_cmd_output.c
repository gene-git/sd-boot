// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Run executable with standard execve() arguments.
 * Child stdout is captured and returned to caller.
 */
#include <fcntl.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sd-boot-cmd.h"


static int init_file_actions(int *fds, posix_spawn_file_actions_t *actions) {
    /*
     * Keep child stdout.
     * - Redirect child's stdout (1) to pipe's write end 
     * - Redirect child stderr to /dev/null
     * - Child doesn't need the to read from it's stdin - close it.
     */
    int ret = 0;

    ret = posix_spawn_file_actions_init(actions);
    if (ret != 0) {
        perror("posix_spawn_file_actions_init");
        return ret;
    }

    /*
     * redirect child stdout to write end of pipe 
     * - fds[1] remains open in child untl spawn is over
    */
    ret = posix_spawn_file_actions_adddup2(actions, fds[1], STDOUT_FILENO);
    if (ret != 0) {
        perror("posix_spawn_file_actions_adddup2");
        return ret;
    }

    /* 
     * Close original pipe write-end
     */
    //posix_spawn_file_actions_addclose(actions, fds[1]);

    /*
     * redirect child stderr to /dev/null
     */
    ret = posix_spawn_file_actions_addopen(actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);
    if (ret != 0) {
        perror("posix_spawn_file_actions_addopen");
        return ret;
    }

    /* 
     * close child read end
     */
    ret = posix_spawn_file_actions_addclose(actions, fds[0]);
    if (ret != 0) {
        perror("posix_spawn_file_actions_addclose");
        return ret;
    }

    return 0;
}


int run_cmd_output(char **argv, char **envp, char **output_p, int *child_ret_p) {
    /*
     * Run command and return allocated string with all child output
     * - if output is null then run command without capturing any
     * output.
     * Args (see man execve or man posix_spawn):
     *  argv - null terminated array
     *  envp - null terminated env array
     * output_p - address pointer to be allocated here 
     *            to hold child stdout (if non-null)
     *            caller must free(*output_p)
     */
    pid_t pid = 0;
    int ret = 0;
    int ret_spawn = 0;
    int fds[2] = { -1, -1 };
    posix_spawnattr_t spawn_attr = {};
    bool spawn_attr_inited = false;
    posix_spawn_file_actions_t actions = {};
    bool actions_inited = false;

    /*
     * sanity check
     */
    if (!envp || !argv || argv[0] == nullptr ||  argv[0][0] == '\0') {
        return -1;
    }

    if (output_p == nullptr) {
        /*
         * no output
         */
        ret = run_cmd(argv, envp, child_ret_p);
        return ret;
    }

    /*
     * good practice to reset child signals
     */
    ret = init_spawn_attr(&spawn_attr);
    if (ret != 0) {
        goto exit;
    }
    spawn_attr_inited = true;

    /*
     * Capture stdout
     * - parent reads fd[0] 
     * - parent (can) write to fd[1]
     */
    if (pipe2(fds, O_CLOEXEC) != 0) {
        perror("pipe failed");
        ret = -1;
        goto exit;
    }

    /*
     * posix file actions:
     *   Keep child stdout 
     * - Redirect child's stdout (1) to pipe's write end 
     * - Redirect child stderr to /dev/null
     * - Child doesn't need to read from it's stdin - so close it.
     *   (we're not writing to it)
     */
    ret = init_file_actions(fds, &actions);
    if (ret != 0) {
        goto exit;
    }
    actions_inited = true;

    /*
     * spawn child process
     */
    ret_spawn = posix_spawn(&pid, argv[0], &actions, &spawn_attr, argv, envp);
    if (ret_spawn != 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    
    /*
     * close parent write since we're not writing to child
     * - a failure here is logged but is not fatal: we still need to
     *   read whatever output is available and reap the child below,
     *   otherwise we'd leak a zombie process and never report child_ret.
     */
    if (close(fds[1]) != 0) {
        perror(nullptr);
        ret = 1;
    }
    fds[1] = -1;

    /*
     * Read stdout output 
     * - this allocates in chunks with extra bytes for null termination
     * - ignore error here since we must wait for child regardless.
     */
    (void)read_child_output(fds[0], output_p);
    (void)close(fds[0]);
    fds[0] = -1;
    
    /*
     * Always Wait for child to exit
     * even if had read errors
     * we check wait returns child pid even though we only wait on that 1 pid.
     */
    int child_ret = 0;
    int child_status = 0;
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
    if (child_ret_p) {
        *child_ret_p = child_ret;
    }

exit:
    if (fds[0] != -1) { 
        close(fds[0]);
    }

    if (fds[1] != -1) {
        close(fds[1]);
    }

    if (actions_inited) {
        if (posix_spawn_file_actions_destroy(&actions) != 0) {
            perror("posix_spawn_file_actions_destroy");
        }
    }

    if (spawn_attr_inited) {
        if (posix_spawnattr_destroy(&spawn_attr) != 0) {
            perror("posix_spawnattr_destroy");
        }
    }
    return ret;
}

