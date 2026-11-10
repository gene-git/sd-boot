// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Run executable with standard execve() arguments.
 * Child stdout and/or stderr are captured and returned to caller.
 *
 * Pieces:
 *   run_cmd_spawn.c     - pipes, file actions, posix_spawn
 *   read_child_output.c - read stdout / stderr (poll, pidfd, timeout)
 *   run_cmd_wait.c      - reap child, terminate on timeout
 *   run_cmd_sigfwd.c    - forward Ctrl-C etc. to child (timeout case)
 */
#include "run_cmd_child.h"
#include "sd-boot-cmd.h"
#include "sd-boot-msg.h"

#include <limits.h>
#include <sys/pidfd.h>

enum OutputConstants {
    MS_PER_SEC = 1000,
};

static int secs_to_ms(int timeout_secs) {
    if (timeout_secs <= 0) {
        return 0;
    }
    return timeout_secs > INT_MAX / MS_PER_SEC ? INT_MAX : timeout_secs * MS_PER_SEC;
}

/*
 * Spawn the child; with a timeout, also set up signal forwarding.
 * On failure everything is cleaned up.
 */
static int start_child(char **argv, char **envp, const SpawnOpts *opts, Child *child, SigForward *fwd) {
    SpawnOpts spawn_opts = *opts;

    if (opts->own_pgroup) {
        if (run_cmd_sigfwd_block(fwd) != 0) {
            return -1;
        }
        spawn_opts.child_sigmask = &fwd->old_mask;
    }

    if (run_cmd_spawn(argv, envp, &spawn_opts, child) != 0) {
        run_cmd_child_close_fds(child);
        run_cmd_sigfwd_stop(fwd);
        return -1;
    }

    /*
     * Not fatal: timeout still works, only Ctrl-C forwarding is lost.
     */
    if (opts->own_pgroup && run_cmd_sigfwd_start(fwd, child->pid) != 0) {
        msg(MSG_ERR, "  ! sd-boot: signal forwarding unavailable\n");
        run_cmd_sigfwd_stop(fwd);
    }

    /*
     * pidfd lets the reader stop when the child exits, even if a
     * grandchild keeps the pipes open. Safe from pid reuse: the child
     * can't be reaped until we wait for it.
     * Optional - on failure (old kernel) we fall back to reading until EOF.
     */
    child->pid_fd = pidfd_open(child->pid, 0);

    /*
     * Close parent copies of the write ends - otherwise we never see EOF.
     */
    run_cmd_close_fd(&child->out_fds[1]);
    run_cmd_close_fd(&child->err_fds[1]);
    return 0;
}

/*
 * Read output until EOF, child exit or timeout; kill child on timeout.
 * Returns 0, -1 (read error) or RUN_CMD_TIMEOUT.
 */
static int collect_output(char **argv, Child *child, int timeout_secs, char **output_p, char **error_p) {
    int ret = read_child_outputs(child->out_fds[0], child->err_fds[0], child->pid_fd,
            secs_to_ms(timeout_secs), output_p, error_p);
    if (ret == RUN_CMD_TIMEOUT) {
        msg(MSG_ERR, "  ! sd-boot: %s timed out after %d seconds - terminating\n", argv[0], timeout_secs);
        run_cmd_terminate(child->pid);
        return RUN_CMD_TIMEOUT;
    }
    return ret == 0 ? 0 : -1;
}

/**
 * Run command and return allocated strings with child output.
 *
 * Args (see man execve or man posix_spawn):
 *  :param argv: Command and args (usual null terminated array)
 *  :param envp: The enviironment (usual null terminated array)
 *  :param output_p: Child's stdout if non-null, allocated and caller must free(*output_p).
 *  :param error_p: Child's stderr if non-null, allocated here Caller must free(*error_p).
 *  :param child_ret_p: Set to child exit status if not null
 *                      If child killed by a signal: 128 + signal number
 *                      (shell convention), e.g. 143 for SIGTERM, 137 for SIGKILL.
 *  :param timeout_secs: If > 0: kill the child (and everything it started) if it
 *                       has not finished in this many seconds, and return
 *                       RUN_CMD_TIMEOUT. Output captured so far is still returned.
 *                       If <= 0: no timeout.
 * :returns: 0 on success else -1.
 *
 * Note:
 * - If both output_p and error_p are null, the command is run
 *   without capturing (run_cmd()).
 * - When capturing, any stream not captured goes to /dev/null.
 * - On success, each requested string is non-null (possibly empty).
 *   On read error they are null and -1 is returned (child is still
 *   reaped and *child_ret_p set).
 * - With a timeout, SIGINT / SIGTERM / SIGHUP / SIGQUIT received while
 *   the child runs are forwarded to it, then re-raised here once it has
 *   been reaped - so by default Ctrl-C still ends everything.
 */
int run_cmd_output(char **argv, char **envp, char **output_p, char **error_p, int *child_ret_p,
        int timeout_secs) {
    Child child = {};
    SigForward fwd = {};
    const SpawnOpts opts = {
        .want_out = output_p != nullptr,
        .want_err = error_p != nullptr,
        .own_pgroup = timeout_secs > 0,
        .child_sigmask = nullptr,
    };

    /*
     * Input sanity check
     */
    if (!envp || !argv || argv[0] == nullptr || argv[0][0] == '\0') {
        return -1;
    }

    if (output_p == nullptr && error_p == nullptr) {
        /*
         * no output requested.
         */
        return run_cmd(argv, envp, child_ret_p);
    }

    if (output_p) {
        *output_p = nullptr;
    }
    if (error_p) {
        *error_p = nullptr;
    }

    run_cmd_child_init(&child);
    if (start_child(argv, envp, &opts, &child, &fwd) != 0) {
        return -1;
    }

    /*
     * A read error is reported but is not fatal: we still reap the child.
     */
    int ret = collect_output(argv, &child, timeout_secs, output_p, error_p);
    run_cmd_child_close_fds(&child);

    int child_ret = run_cmd_wait(child.pid);
    if (child_ret_p) {
        *child_ret_p = child_ret;
    }

    /*
     * Last: may re-raise a forwarded signal and not return.
     */
    run_cmd_sigfwd_stop(&fwd);
    return ret;
}
