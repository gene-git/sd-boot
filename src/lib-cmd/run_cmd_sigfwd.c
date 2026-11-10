// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Forward terminal / termination signals to a child's process group.
 *
 * When run_cmd_output() has a timeout, the child runs in its own process
 * group so it (and everything it starts) can be killed on timeout. That
 * also takes it out of the terminal's foreground group, so Ctrl-C would
 * only reach us. While the child runs we catch SIGINT, SIGTERM, SIGHUP and
 * SIGQUIT and send the same signal to the child's group. Once the child is
 * reaped, previous handlers are restored and the signal is re-raised so we
 * behave as if we'd received it directly (default: terminate).
 *
 * Usage:
 *   run_cmd_sigfwd_block()  - before spawn: block the signals so one
 *                             arriving before handlers are in place is
 *                             held, not lost.
 *   run_cmd_sigfwd_start()  - after spawn: install handlers, unblock.
 *   run_cmd_sigfwd_stop()   - after child reaped: restore, re-raise.
 *                             May not return (if signal is fatal).
 *
 * nb: signal handlers are process wide; the signal mask (pthread_sigmask)
 * is per thread. Fine for a single-threaded tool. In a threaded program
 * the handler still forwards, but only this thread's mask is changed.
 * Not re-entrant (one forwarded child at a time).
 */
#include "run_cmd_child.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>

static const int fwd_signals[NUM_FWD_SIGNALS] = {SIGINT, SIGTERM, SIGHUP, SIGQUIT};

/*
 * Shared with the signal handler - a handler can't take arguments,
 * so file-scope state is the only option.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static volatile sig_atomic_t fwd_pgid = 0;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static volatile sig_atomic_t fwd_sig = 0;

/*
 * Async-signal-safe: only kill() and sig_atomic_t stores.
 */
static void forward_signal(int sig) {
    int saved_errno = errno;

    fwd_sig = sig;
    if (fwd_pgid > 0) {
        (void)kill(-(pid_t)fwd_pgid, sig);
    }
    errno = saved_errno;
}

static void fwd_sigset(sigset_t *set) {
    (void)sigemptyset(set);
    for (int i = 0; i < NUM_FWD_SIGNALS; i++) {
        (void)sigaddset(set, fwd_signals[i]);
    }
}

/*
 * pthread_sigmask: MT-safe equivalent of sigprocmask.
 * Returns the error number (does not set errno).
 */
static int set_sigmask(int how, const sigset_t *set, sigset_t *old_set, const char *what) {
    int ret = pthread_sigmask(how, set, old_set);
    if (ret != 0) {
        errno = ret;
        perror(what);
        return -1;
    }
    return 0;
}

int run_cmd_sigfwd_block(SigForward *fwd) {
    sigset_t set = {};

    *fwd = (SigForward){};
    fwd_sigset(&set);
    if (set_sigmask(SIG_BLOCK, &set, &fwd->old_mask, "pthread_sigmask block") != 0) {
        return -1;
    }
    fwd->mask_saved = true;
    return 0;
}

int run_cmd_sigfwd_start(SigForward *fwd, pid_t pgid) {
    struct sigaction action = {};

    if (!fwd->mask_saved) {
        return -1;
    }

    action.sa_handler = forward_signal;
    action.sa_flags = SA_RESTART;
    (void)sigfillset(&action.sa_mask);

    fwd_sig = 0;
    fwd_pgid = (sig_atomic_t)pgid;

    for (int i = 0; i < NUM_FWD_SIGNALS; i++) {
        if (sigaction(fwd_signals[i], &action, &fwd->old_actions[i]) != 0) {
            perror("sigaction");
            return -1;
        }
        fwd->num_installed = i + 1;
    }

    /*
     * Unblock: anything that arrived since run_cmd_sigfwd_block()
     * is delivered now - and forwarded.
     */
    if (set_sigmask(SIG_SETMASK, &fwd->old_mask, nullptr, "pthread_sigmask unblock") != 0) {
        return -1;
    }
    return 0;
}

void run_cmd_sigfwd_stop(SigForward *fwd) {
    sigset_t set = {};

    if (!fwd->mask_saved) {
        return;
    }

    /*
     * Block while swapping handlers back so nothing slips through
     * to a half-restored state. Anything arriving now stays pending
     * and goes to the restored handler when we unblock.
     */
    fwd_sigset(&set);
    (void)set_sigmask(SIG_BLOCK, &set, nullptr, "pthread_sigmask block");

    for (int i = 0; i < fwd->num_installed; i++) {
        (void)sigaction(fwd_signals[i], &fwd->old_actions[i], nullptr);
    }
    int sig = (int)fwd_sig;
    fwd_pgid = 0;
    fwd_sig = 0;

    (void)set_sigmask(SIG_SETMASK, &fwd->old_mask, nullptr, "pthread_sigmask restore");
    fwd->mask_saved = false;
    fwd->num_installed = 0;

    /*
     * Act on the signal ourselves, as if it had come straight to us.
     */
    if (sig != 0) {
        (void)raise(sig);
    }
}
