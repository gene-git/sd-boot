// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot-cmd.h
 */
#ifndef SD_BOOT_CMD_H
#define SD_BOOT_CMD_H

#include <limits.h>
#include <spawn.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * Function declarations
 */
int init_spawn_attr(posix_spawnattr_t *attr);
int read_child_output(int fdes, char **output_p);

int run_cmd(char **argv, char **envp, int *child_ret_p);
int run_cmd_output(char **argv, char **envp, char **output_p, int *child_ret_p);

#endif
