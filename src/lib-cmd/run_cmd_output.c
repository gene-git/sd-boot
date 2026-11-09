// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Run executable with standard execve() arguments.
 * Child stdout and/or stderr are captured and returned to caller.
 */
#include "sd-boot-cmd.h"
#include "sd-boot-msg.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sched.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/pidfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

enum RunCmdConstants {
    MS_PER_SEC = 1000,
    NS_PER_MS = 1000000,
    KILL_GRACE_MS = 5000,       /* SIGTERM -> SIGKILL */
    KILL_POLL_MS = 100,
    SIGNAL_EXIT_BASE = 128,     /* shell convention: killed by signal N -> 128 + N */
};

/*
 * Child process and the parent's ends of its pipes.
 * - [0] read end (parent), [1] write end (child)
 */
typedef struct {
    pid_t pid;
    int out_fds[2];
    int err_fds[2];
    int pid_fd;
} Child;

static void close_fd(int *fdes) {
    if (*fdes >= 0) {
        (void)close(*fdes);
        *fdes = -1;
    }
}

static void child_close_fds(Child *child) {
    close_fd(&child->pid_fd);
    close_fd(&child->out_fds[0]);
    close_fd(&child->out_fds[1]);
    close_fd(&child->err_fds[0]);
    close_fd(&child->err_fds[1]);
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
    if (ret != 0) {
        errno = ret;
        perror("posix_spawn_file_actions");
        (void)posix_spawn_file_actions_destroy(actions);
        return -1;
    }
    return 0;
}

/*
 * Put child in its own process group so on timeout we can kill
 * everything it started, not just the top process.
 */
static int spawn_attr_set_pgroup(posix_spawnattr_t *spawn_attr) {
    short flags = 0;
    int ret = posix_spawnattr_getflags(spawn_attr, &flags);
    if (ret == 0) {
        unsigned int new_flags = (unsigned int)flags | (unsigned int)POSIX_SPAWN_SETPGROUP;
        ret = posix_spawnattr_setflags(spawn_attr, (short)new_flags);
    }
    if (ret == 0) {
        ret = posix_spawnattr_setpgroup(spawn_attr, 0);
    }
    if (ret != 0) {
        errno = ret;
        perror("posix_spawnattr pgroup");
        return -1;
    }
    return 0;
}

/*
 * One pipe per captured stream.
 */
static int open_pipes(bool want_out, bool want_err, Child *child) {
    if (want_out && pipe2(child->out_fds, O_CLOEXEC) != 0) {
        perror("pipe2 stdout");
        return -1;
    }
    if (want_err && pipe2(child->err_fds, O_CLOEXEC) != 0) {
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
static int spawn_child(char **argv, char **envp, bool want_out, bool want_err, bool own_pgroup, Child *child) {
    int ret = -1;
    int ret_spawn = 0;
    posix_spawnattr_t spawn_attr = {};
    posix_spawn_file_actions_t actions = {};

    /*
     * good practice to reset child signals
     */
    if (init_spawn_attr(&spawn_attr) != 0) {
        return -1;
    }

    /*
     * Own process group only when we may need to kill it.
     * nb: the child is then not in the terminal's foreground group, so
     * Ctrl-C reaches us but not the child.
     */
    if ((own_pgroup && spawn_attr_set_pgroup(&spawn_attr) != 0) || open_pipes(want_out, want_err, child) != 0) {
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

/*
 * Has child exited? Does not reap it (WNOWAIT) - waitpid() does that later.
 */
static bool child_exited(pid_t pid) {
    siginfo_t info = {};
    if (waitid(P_PID, (id_t)pid, &info, WEXITED | WNOHANG | WNOWAIT) != 0) {
        return errno != EINTR;      /* ECHILD etc - nothing to wait for */
    }
    return info.si_pid == pid;
}

/*
 * Timed out: SIGTERM the child's whole process group (dracut, ukify,
 * sbsign etc. run as grandchildren), give it KILL_GRACE_MS to clean up,
 * then SIGKILL.
 * Child was spawned with POSIX_SPAWN_SETPGROUP so its pgid == pid.
 */
static void terminate_child(pid_t pid) {
    const struct timespec nap = { .tv_sec = 0, .tv_nsec = (long)KILL_POLL_MS * NS_PER_MS };

    (void)kill(-pid, SIGTERM);
    for (int waited = 0; waited < KILL_GRACE_MS; waited += KILL_POLL_MS) {
        if (child_exited(pid)) {
            /*
             * Child is gone - grandchildren may linger; SIGKILL them.
             */
            break;
        }
        (void)nanosleep(&nap, nullptr);
    }
    (void)kill(-pid, SIGKILL);
}

/*
 * Reap child. Returns exit status, 128 + signal if killed by a signal,
 * or -1 if waitpid failed.
 */
static int wait_child(pid_t pid) {
    int child_status = 0;
    pid_t wait_ret = 0;

    do {
        wait_ret = waitpid(pid, &child_status, 0);
    } while (wait_ret < 0 && errno == EINTR);

    if (wait_ret != pid) {
        perror("waitpid");
        return -1;
    }
    if (WIFEXITED(child_status) != 0) {
        return WEXITSTATUS(child_status);
    }
    if (WIFSIGNALED(child_status) != 0) {
        return SIGNAL_EXIT_BASE + WTERMSIG(child_status);
    }
    return 0;
}

static int secs_to_ms(int timeout_secs) {
    if (timeout_secs <= 0) {
        return 0;
    }
    return timeout_secs > INT_MAX / MS_PER_SEC ? INT_MAX : timeout_secs * MS_PER_SEC;
}

/*
 * Read output until EOF, child exit or timeout; kill child on timeout.
 * Returns 0, -1 (read error) or RUN_CMD_TIMEOUT.
 */
static int collect_output(char **argv, Child *child, int timeout_secs, char **output_p, char **error_p) {
    int ret = read_child_outputs(child->out_fds[0], child->err_fds[0], child->pid_fd, secs_to_ms(timeout_secs),
                                 output_p, error_p);
    if (ret == RUN_CMD_TIMEOUT) {
        msg(MSG_ERR, "  ! sd-boot: %s timed out after %d seconds - terminating\n", argv[0], timeout_secs);
        terminate_child(child->pid);
        return RUN_CMD_TIMEOUT;
    }
    return ret == 0 ? 0 : -1;
}

int run_cmd_output(char **argv, char **envp, char **output_p, char **error_p, int *child_ret_p,
                   int timeout_secs) {
    /*
     * Run command and return allocated strings with child output.
     *
     * Args (see man execve or man posix_spawn):
     *  argv    - null terminated array
     *  envp    - null terminated env array
     *  output_p - if non-null, *output_p is allocated here and holds
     *             child stdout. Caller must free(*output_p).
     *  error_p  - if non-null, *error_p is allocated here and holds
     *             child stderr. Caller must free(*error_p).
     *  child_ret_p - if non-null, set to child exit status.
     *             If child was killed by a signal: 128 + signal number
     *             (shell convention), e.g. 143 for SIGTERM, 137 for SIGKILL.
     *  timeout_secs - > 0: kill the child (and everything it started) if it
     *             has not finished in this many seconds, and return
     *             RUN_CMD_TIMEOUT. Output captured so far is still returned.
     *             <= 0: no timeout.
     *
     * - If both output_p and error_p are null, the command is run
     *   without capturing (run_cmd()).
     * - When capturing, any stream not captured goes to /dev/null.
     * - On success, each requested string is non-null (possibly empty).
     *   On read error they are null and -1 is returned (child is still
     *   reaped and *child_ret_p set).
     */
    Child child = {
        .pid = 0,
        .out_fds = { -1, -1 },
        .err_fds = { -1, -1 },
        .pid_fd = -1,
    };

    /*
     * sanity check
     */
    if (!envp || !argv || argv[0] == nullptr || argv[0][0] == '\0') {
        return -1;
    }

    if (output_p == nullptr && error_p == nullptr) {
        /*
         * no output
         */
        return run_cmd(argv, envp, child_ret_p);
    }

    if (output_p) {
        *output_p = nullptr;
    }
    if (error_p) {
        *error_p = nullptr;
    }

    if (spawn_child(argv, envp, output_p != nullptr, error_p != nullptr, timeout_secs > 0, &child) != 0) {
        child_close_fds(&child);
        return -1;
    }

    /*
     * pidfd lets the reader stop when the child exits, even if a
     * grandchild keeps the pipes open. Safe from pid reuse: the child
     * can't be reaped until we waitpid() below.
     * Optional - on failure (old kernel) we fall back to reading until EOF.
     */
    child.pid_fd = pidfd_open(child.pid, 0);

    /*
     * Close parent copies of the write ends - otherwise we never see EOF.
     */
    close_fd(&child.out_fds[1]);
    close_fd(&child.err_fds[1]);

    /*
     * A read error is reported but is not fatal here: we must still reap the child.
     */
    int ret = collect_output(argv, &child, timeout_secs, output_p, error_p);
    child_close_fds(&child);

    int child_ret = wait_child(child.pid);
    if (child_ret_p) {
        *child_ret_p = child_ret;
    }
    return ret;
}
