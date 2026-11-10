// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Spawn a child for run_cmd_output with its stdout / stderr
 * connected to pipes (or /dev/null) and stdin from /dev/null.
 */
#include "run_cmd_child.h"
#include "sd-boot-cmd.h"

#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <unistd.h>

void run_cmd_child_init(Child *child) {
    *child = (Child){
        .pid = 0,
        .out_fds = {-1, -1},
        .err_fds = {-1, -1},
        .pid_fd = -1,
    };
}

void run_cmd_close_fd(int *fdes) {
    if (*fdes >= 0) {
        (void)close(*fdes);
        *fdes = -1;
    }
}

void run_cmd_child_close_fds(Child *child) {
    run_cmd_close_fd(&child->pid_fd);
    run_cmd_close_fd(&child->out_fds[0]);
    run_cmd_close_fd(&child->out_fds[1]);
    run_cmd_close_fd(&child->err_fds[0]);
    run_cmd_close_fd(&child->err_fds[1]);
}

/*
 * Child file actions.
 * stdin: /dev/null - a child that tries to read input (e.g. a prompt)
 *        gets EOF immediately instead of blocking on our terminal.
 * For each of stdout / stderr:
 * - if captured: dup2 the pipe write end onto it.
 * - otherwise:   redirect it to /dev/null.
 *
 * Pipes are created O_CLOEXEC, so the original pipe fds (both ends)
 * are closed automatically in the child on exec - only the dup2'd
 * copies on 1 / 2 survive (dup2 clears FD_CLOEXEC on the target).
 *
 * Finally close every other inherited fd (>= 3): anything the parent
 * opened without O_CLOEXEC would otherwise leak into the child and
 * everything it runs. Must be last - actions run in order, and the
 * pipe fds are needed by the dup2 actions above.
 * (posix_spawn_file_actions_addclosefrom_np: glibc >= 2.34)
 */
static int add_output_action(posix_spawn_file_actions_t *actions, int pipe_w, int target_fd) {
    if (pipe_w >= 0) {
        return posix_spawn_file_actions_adddup2(actions, pipe_w, target_fd);
    }
    return posix_spawn_file_actions_addopen(actions, target_fd, "/dev/null", O_WRONLY, 0);
}

static int init_file_actions(int out_w, int err_w, posix_spawn_file_actions_t *actions) {
    int ret = posix_spawn_file_actions_init(actions);
    if (ret != 0) {
        errno = ret;
        perror("posix_spawn_file_actions_init");
        return -1;
    }

    ret = posix_spawn_file_actions_addopen(actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    if (ret == 0) {
        ret = add_output_action(actions, out_w, STDOUT_FILENO);
    }
    if (ret == 0) {
        ret = add_output_action(actions, err_w, STDERR_FILENO);
    }
    if (ret == 0) {
        ret = posix_spawn_file_actions_addclosefrom_np(actions, STDERR_FILENO + 1);
    }
    if (ret != 0) {
        errno = ret;
        perror("posix_spawn_file_actions");
        (void)posix_spawn_file_actions_destroy(actions);
        return -1;
    }
    return 0;
}

/*
 * OR extra flags into spawn attributes.
 * Reports whether 'check' was already set (if non-null).
 */
static int spawn_attr_add_flags(posix_spawnattr_t *spawn_attr, short add, short check, bool *had_check) {
    short flags = 0;
    int ret = posix_spawnattr_getflags(spawn_attr, &flags);
    if (ret != 0) {
        return ret;
    }
    if (had_check) {
        *had_check = ((unsigned int)flags & (unsigned int)check) != 0U;
    }
    unsigned int new_flags = (unsigned int)flags | (unsigned int)add;
    return posix_spawnattr_setflags(spawn_attr, (short)new_flags);
}

/*
 * Optional attributes on top of init_spawn_attr():
 * - own process group: so on timeout we can kill everything the child
 *   started, not just the top process.
 * - signal mask: the parent may have signals blocked while spawning
 *   (signal forwarding). Unless init_spawn_attr() already set a mask,
 *   give the child the caller's mask so it doesn't inherit the block.
 */
static int spawn_attr_extras(posix_spawnattr_t *spawn_attr, const SpawnOpts *opts) {
    int ret = 0;

    if (opts->own_pgroup) {
        ret = spawn_attr_add_flags(spawn_attr, POSIX_SPAWN_SETPGROUP, 0, nullptr);
        if (ret == 0) {
            ret = posix_spawnattr_setpgroup(spawn_attr, 0);
        }
    }

    if (ret == 0 && opts->child_sigmask) {
        bool had_mask = false;
        ret = spawn_attr_add_flags(spawn_attr, POSIX_SPAWN_SETSIGMASK, POSIX_SPAWN_SETSIGMASK, &had_mask);
        if (ret == 0 && !had_mask) {
            ret = posix_spawnattr_setsigmask(spawn_attr, opts->child_sigmask);
        }
    }

    if (ret != 0) {
        errno = ret;
        perror("posix_spawnattr");
        return -1;
    }
    return 0;
}

/*
 * One pipe per captured stream.
 */
static int open_pipes(const SpawnOpts *opts, Child *child) {
    if (opts->want_out && pipe2(child->out_fds, O_CLOEXEC) != 0) {
        perror("pipe2 stdout");
        return -1;
    }
    if (opts->want_err && pipe2(child->err_fds, O_CLOEXEC) != 0) {
        perror("pipe2 stderr");
        return -1;
    }
    return 0;
}

/*
 * Create pipes and spawn the child.
 * On failure caller still closes child fds.
 * - posix_spawn returns the error number; it does not set errno.
 */
int run_cmd_spawn(char **argv, char **envp, const SpawnOpts *opts, Child *child) {
    int ret = -1;
    int ret_spawn = 0;
    posix_spawnattr_t spawn_attr = {};
    posix_spawn_file_actions_t actions = {};

    /*
     * good practice to reset child signals
     * - attr is destroyed even if init fails (init_spawn_attr contract)
     */
    if (init_spawn_attr(&spawn_attr) != 0) {
        goto exit_attr;
    }

    if (spawn_attr_extras(&spawn_attr, opts) != 0 || open_pipes(opts, child) != 0) {
        goto exit_attr;
    }

    if (init_file_actions(child->out_fds[1], child->err_fds[1], &actions) != 0) {
        goto exit_attr;
    }

    ret_spawn = posix_spawn(&child->pid, argv[0], &actions, &spawn_attr, argv, envp);
    if (ret_spawn != 0) {
        errno = ret_spawn;
        perror(argv[0]);
    } else {
        ret = 0;
    }

    (void)posix_spawn_file_actions_destroy(&actions);
exit_attr:
    (void)posix_spawnattr_destroy(&spawn_attr);
    return ret;
}
