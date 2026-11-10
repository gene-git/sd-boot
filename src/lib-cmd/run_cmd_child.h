// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * run_cmd_child.h
 *
 * Internal to lib-cmd: helpers shared by run_cmd_output.c,
 * run_cmd_spawn.c, run_cmd_wait.c and run_cmd_sigfwd.c.
 * Not part of the public API (see sd-boot-cmd.h).
 */
#ifndef RUN_CMD_CHILD_H
#define RUN_CMD_CHILD_H

#include <signal.h>

/*
 * Child process and the pipes / pidfd used to talk to it.
 * - pipes: [0] read end (parent), [1] write end (child)
 * - unused fds are -1
 */
typedef struct {
    pid_t pid;
    int out_fds[2];
    int err_fds[2];
    int pid_fd;
} Child;

/*
 * How to spawn the child.
 * - want_out / want_err: create a pipe for stdout / stderr
 *                        (otherwise that stream goes to /dev/null)
 * - own_pgroup:   child gets its own process group (pgid == pid)
 * - child_sigmask: if non-null, child's signal mask, used when
 *                  init_spawn_attr() did not set one. Needed when the
 *                  parent has signals blocked at spawn time.
 */
typedef struct {
    bool want_out;
    bool want_err;
    bool own_pgroup;
    const sigset_t *child_sigmask;
} SpawnOpts;

/*
 * Signal forwarding state - see run_cmd_sigfwd.c
 */
enum SigFwdConstants {
    NUM_FWD_SIGNALS = 4,
};

typedef struct {
    struct sigaction old_actions[NUM_FWD_SIGNALS];
    int num_installed;
    sigset_t old_mask;
    bool mask_saved;
} SigForward;

/* run_cmd_spawn.c */
void run_cmd_child_init(Child *child);
void run_cmd_close_fd(int *fdes);
void run_cmd_child_close_fds(Child *child);
int run_cmd_spawn(char **argv, char **envp, const SpawnOpts *opts, Child *child);

/* run_cmd_wait.c */
void run_cmd_terminate(pid_t pid);
int run_cmd_wait(pid_t pid);

/* run_cmd_sigfwd.c */
int run_cmd_sigfwd_block(SigForward *fwd);
int run_cmd_sigfwd_start(SigForward *fwd, pid_t pgid);
void run_cmd_sigfwd_stop(SigForward *fwd);

#endif
