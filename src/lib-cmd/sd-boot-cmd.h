// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-cmd.h
 */
#ifndef SD_BOOT_CMD_H
#define SD_BOOT_CMD_H

#include <limits.h>
#include <spawn.h>
#include <stddef.h>
#include <stdint.h>

enum RunCmdStatus { RUN_CMD_TIMEOUT = -2 };

/*
 * Function declarations
 */
int init_spawn_attr(posix_spawnattr_t *attr);
int read_child_output(int fdes, char **output_p);
int read_child_outputs(int out_fd, int err_fd, int pid_fd, int timeout_ms,
                       char **output_p, char **error_p);

int run_cmd(char **argv, char **envp, int *child_ret_p);
int run_cmd_output_show(char **argv, char **envp, int *child_ret_p, uint16_t margin, int line_len,
                        int timeout_secs);
int run_cmd_output(char **argv, char **envp, char **output_p, char **error_p, int *child_ret_p,
                   int timeout_secs);
#endif
