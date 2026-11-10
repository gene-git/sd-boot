// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Reap or terminate a child spawned by run_cmd_output.
 */
#include "run_cmd_child.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

enum WaitConstants {
    NS_PER_MS = 1000000,
    KILL_GRACE_MS = 5000, /* SIGTERM -> SIGKILL */
    KILL_POLL_MS = 100,
    SIGNAL_EXIT_BASE = 128, /* shell convention: killed by signal N -> 128 + N */
};

/*
 * Has child exited? Does not reap it (WNOWAIT) - run_cmd_wait() does that.
 */
static bool child_exited(pid_t pid) {
    siginfo_t info = {};
    if (waitid(P_PID, (id_t)pid, &info, WEXITED | WNOHANG | WNOWAIT) != 0) {
        return errno != EINTR; /* ECHILD etc - nothing to wait for */
    }
    return info.si_pid == pid;
}

/*
 * Timed out: SIGTERM the child's whole process group (dracut, ukify,
 * sbsign etc. run as grandchildren), give it KILL_GRACE_MS to clean up,
 * then SIGKILL.
 * Child was spawned with its own process group so its pgid == pid.
 */
void run_cmd_terminate(pid_t pid) {
    const struct timespec nap = {.tv_sec = 0, .tv_nsec = (long)KILL_POLL_MS * NS_PER_MS};

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
int run_cmd_wait(pid_t pid) {
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
