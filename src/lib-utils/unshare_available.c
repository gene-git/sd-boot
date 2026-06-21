// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Check if "unshare" is available. See man 2 unshare.
 *
 * Even if host system allows it (sysctl user.max_user_namespaces returns number > 0)
 * If the test is already running in a container or chroot (as arch package builds often do)
 * unshare may not work. Testig will fall back to using env variables.
 *
 * Requiores _GNU_SOURCE
 *
 * Return true if unshare is availabl
 */
#include <sched.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Check by fork child which attempts to create a new namespace
 */
bool unshare_available() {
    pid_t pid = fork();

    if (pid < 0) {
        return false;
    }

    if (pid == 0) {
        if (unshare(CLONE_NEWUSER | CLONE_NEWNS) == 0) {
            _exit(0);
        } else {
            _exit(1);
        }
    } else {
        int wstatus = -1;

        if (waitpid(pid, &wstatus, 0) < 0) {
            return false;
        }

        if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 0) {
            return true;
        }
    }
    return false;
} 
