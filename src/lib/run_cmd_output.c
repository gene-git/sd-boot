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
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sd-boot.h"

enum BufSize {
    CHUNK = 4096,
};

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
        perror(nullptr);
        goto exit;
    }

    /*
     * redirect child stdout to write end of pipe 
     * - fds[1] remains open in child untl spawn is over
    */
    ret = posix_spawn_file_actions_adddup2(actions, fds[1], STDOUT_FILENO);
    if (ret != 0) {
        perror(nullptr);
        goto exit;
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
        perror(nullptr);
        goto exit;
    }

    /* 
     * close child read end
     */
    posix_spawn_file_actions_addclose(actions, fds[0]);


exit:
    if (ret != 0) {
        if (posix_spawn_file_actions_destroy(actions) != 0) {
            perror(nullptr);
        }
    }
    return ret;
}

static int read_child_output(int fdes, char **output_p) {
    /*
     * Read child stdout and save into *output_p
     * - allocate in chunks.
     * - allocate one extra byte leaving room for null termination.
     * We realloc() mem to what is needed - if no output then 
     * memory is freed and ptr set to null.
     */
    int ret = 0;
    size_t one_byte = sizeof(char);
    char *ptr = nullptr;
    Dynamic_str str = {};

    /*
     * sanity
     */
    if (!output_p) {
        msg(MSG_ERR, "  ! read_child: bad output ptr\n");
        ret = -1;
        goto exit;
    }

    /*
     * First chunk of memory
     */
    *output_p = nullptr;
    //bytes_allocated = chunk;
    //num_alloc = CHUNK;
    ret = dynamic_str_alloc(CHUNK, &str);
    if (ret != 0) {
        goto exit;
    }

    /*
     * Ptr tracks where in the buffer to read the next chunk.
     */
    ptr = str.bytes;
    *ptr = '\0';
    ssize_t bytes_read = 0;
    while ((bytes_read = read(fdes, ptr, CHUNK)) > 0) {
        str.num_used += (size_t)bytes_read / one_byte;

        ret = dynamic_str_alloc(str.num_used + CHUNK, &str);
        if (ret != 0) {
            goto exit;
        }
        ptr = str.bytes + str.num_used;
    }

    /*
     * Resize down add null terminator
     */
    if (str.num_used < str.num_alloc + 1) {
        ret = dynamic_str_alloc(str.num_used + 1, &str);
        if (ret != 0) {
            goto exit;
        }
    } 

    if (str.bytes != nullptr && str.bytes[str.num_used] != '\0') {
        str.bytes[str.num_used] = '\0';
    }

    *output_p = str.bytes;
    str.bytes = nullptr;
    str.num_alloc = 0;

exit:
    if (str.bytes) {
        /*
         * error where pointer is not transferred to *output_p 
         */
        (void)dynamic_str_alloc(0, &str);
    }
    return ret;
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
    int fds[2] = {};

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
     * Capture stdout
     * - parent reads fd[0] 
     * - parent (can) write to fd[1]
     */
    if (pipe2(fds, O_CLOEXEC) != 0) {
        perror("pipe failed");
        return -1;
    }

    /*
     * posix file actions:
     *   Keep child stdout 
     * - Redirect child's stdout (1) to pipe's write end 
     * - Redirect child stderr to /dev/null
     * - Child doesn't need to read from it's stdin - so close it.
     *   (we're not writing to it)
     */
    posix_spawn_file_actions_t actions = {};
    ret = init_file_actions(fds, &actions);
    if (ret != 0) {
        goto exit;
    }

    /*
     * spawn child process
     */
    ret_spawn = posix_spawn(&pid, argv[0], &actions, nullptr, argv, envp);
    if (ret_spawn != 0) {
        perror(nullptr);
        ret = -1;
        goto exit;
    }
    
    /*
     * close parent write since we're not writing to child
     */
    if (close(fds[1]) != 0) {
        perror(nullptr);
        ret = 1;
        goto exit;
    }

    /*
     * Read stdout output 
     * - this allocates in chunks with extra bytes for null termination
     * - ignore error here since we must wait for child regardless.
     */
    (void)read_child_output(fds[0], output_p);
    (void)close(fds[0]);
    
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
    if (posix_spawn_file_actions_destroy(&actions) != 0) {
        perror(nullptr);
    }

    return ret;
}
