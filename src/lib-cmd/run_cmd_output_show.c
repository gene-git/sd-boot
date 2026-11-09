// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: © 2026-present Gene C <arch@sapience.com>
/**
 * Run executable and display it's stdout on current process stdout.
 * Allow output to be shifted right to faciliate formattig.
 * Ouptut is only shown at verb level 2 or higher.
 * Variation of run_cmd(), run_cms_output()
 *
 * :param argv: What to run.
 * :param envp: The environ to use
 * :param child_ret_p: On return is the child process exit code.
 * :param margin: Number of spaces to prefix the output
 * :param line_len: The max line length beyond the margin
 * :param timeout: If > 0, kill the child process group if it has not completed in this time.
 */
#include "sd-boot-cmd.h"
#include "sd-boot-msg.h"
#include <stdint.h>
#include <stdlib.h>


int run_cmd_output_show(char **argv, char **envp, int *child_ret_p, uint16_t margin, 
        int line_len, int timeout_secs) {
    int ret = 0;
    char *output = nullptr;
    char *error = nullptr;

    ret = run_cmd_output(argv, envp, &output, &error, child_ret_p, timeout_secs);
    if (ret != 0) {
        goto exit;
    }

    if (!output || output[0] == 0) {
        goto exit;
    }

    msg_buf(MSG_NORMAL, margin, line_len, output);
    msg_buf(MSG_NORMAL, margin, line_len, error);

exit:
    if (output) {
        free((void *)output);
    }
    if (error) {
        free((void *)error);
    }
    return ret;
}
