// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * sd-boot.h
 */
#ifndef SD_BOOT_CMD_H
#define SD_BOOT_CMD_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * Function declarations
 */
int run_cmd(char **argv, char **envp, int *child_ret_p);
int run_cmd_output(char **argv, char **envp, char **output_p, int *child_ret_p);

#endif
